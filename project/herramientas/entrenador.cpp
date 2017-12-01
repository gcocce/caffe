#include <gflags/gflags.h>
#include <glog/logging.h>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "caffe/caffe.hpp"
#include "boost/algorithm/string.hpp"
#include "caffe/util/signal_handler.h"

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::Layer;
using caffe::Solver;
using caffe::shared_ptr;
using caffe::string;
using caffe::Timer;
using caffe::vector;
using std::ostringstream;

/** Defines implementados usando la librería gflags */
DEFINE_string(solver, "",
    "El archivo con la configuración del Solver en formato google protocol buffer.");
DEFINE_string(model, "",
    "La definicion del modelo en formato google protocol buffer.");
DEFINE_string(phase, "",
    "Opcional; fase de la red (TRAIN o TEST). Solo para la opción 'time'.");
DEFINE_string(snapshot, "", "Opcional; imagen del estado de la red para continuar el entrenamiento.");
DEFINE_string(weights, "", "Opcional; parametros para inicializar un proceso de refinamiento, "
    "seprados por ','. No pueden darse al mismo tiempo que un snapshot.");
DEFINE_int32(iterations, 50,"Número de iteraciones a ejecutar.");
DEFINE_string(sigint_effect, "stop",
             "Opcional; accion a ejecutar al recibir la señal SIGINT: "
              "snapshot, stop o none.");
DEFINE_string(sighup_effect, "snapshot",
             "Opcional; acción a ejecutar al recibir la señal SIGHUP: "
             "snapshot, stop or none.");

// Un registro de los comandos de la herramienta.
typedef int (*BrewFunction)();
typedef std::map<caffe::string, BrewFunction> BrewMap;
BrewMap g_brew_map;

#define RegisterBrewFunction(func) \
namespace { \
class __Registerer_##func { \
 public: /* NOLINT */ \
  __Registerer_##func() { \
    g_brew_map[#func] = &func; \
  } \
}; \
__Registerer_##func g_registerer_##func; \
}

static BrewFunction GetBrewFunction(const caffe::string& name) {
  if (g_brew_map.count(name)) {
    return g_brew_map[name];
  } else {
    LOG(ERROR) << "Available caffe actions:";
    for (BrewMap::iterator it = g_brew_map.begin();
         it != g_brew_map.end(); ++it) {
      LOG(ERROR) << "\t" << it->first;
    }
    LOG(FATAL) << "Unknown action: " << name;
    return NULL;
  }
}

// Se obtiene la fase de la configuración del modelo para el entrenamiento
caffe::Phase get_phase_from_flags(caffe::Phase default_value) {
  if (FLAGS_phase == "")
    return default_value;
  if (FLAGS_phase == "TRAIN")
    return caffe::TRAIN;
  if (FLAGS_phase == "TEST")
    return caffe::TEST;
  LOG(FATAL) << "phase must be \"TRAIN\" or \"TEST\"";
  return caffe::TRAIN; 
}

// Se cargan los parametros del modelo para entrenamiento y testing
void CopyLayers(caffe::Solver<float>* solver, const std::string& model_list) {
  std::vector<std::string> model_names;
  boost::split(model_names, model_list, boost::is_any_of(",") );
  for (int i = 0; i < model_names.size(); ++i) {
    LOG(INFO) << "Finetuning from " << model_names[i];
    solver->net()->CopyTrainedLayersFrom(model_names[i]);
    for (int j = 0; j < solver->test_nets().size(); ++j) {
      solver->test_nets()[j]->CopyTrainedLayersFrom(model_names[i]);
    }
  }
}

caffe::SolverAction::Enum GetRequestedAction(
    const std::string& flag_value) {
  if (flag_value == "stop") {
    return caffe::SolverAction::STOP;
  }
  if (flag_value == "snapshot") {
    return caffe::SolverAction::SNAPSHOT;
  }
  if (flag_value == "none") {
    return caffe::SolverAction::NONE;
  }
  LOG(FATAL) << "Invalid signal effect \""<< flag_value << "\" was specified";
}

// Entrenar o refinar un modelo
int train() {
  CHECK_GT(FLAGS_solver.size(), 0) << "Need a solver definition to train.";
  CHECK(!FLAGS_snapshot.size() || !FLAGS_weights.size())
      << "Give a snapshot to resume training or weights to finetune "
      "but not both.";

  caffe::SolverParameter solver_param;
  caffe::ReadSolverParamsFromTextFileOrDie(FLAGS_solver, &solver_param);

  solver_param.mutable_train_state()->set_level(0);

  LOG(INFO) << "Use CPU.";
  Caffe::set_mode(Caffe::CPU);

  caffe::SignalHandler signal_handler(
        GetRequestedAction(FLAGS_sigint_effect),
        GetRequestedAction(FLAGS_sighup_effect));

  shared_ptr<caffe::Solver<float> >
      solver(caffe::SolverRegistry<float>::CreateSolver(solver_param));

  solver->SetActionFunction(signal_handler.GetActionFunction());

  if (FLAGS_snapshot.size()) {
    LOG(INFO) << "Resuming from " << FLAGS_snapshot;
    solver->Restore(FLAGS_snapshot.c_str());
  } else if (FLAGS_weights.size()) {
    CopyLayers(solver.get(), FLAGS_weights);
  }

  LOG(INFO) << "Se inicia la resolución";
  solver->Solve();
  LOG(INFO) << "Fin resolución.";
  return 0;
}
RegisterBrewFunction(train);


// Test: determina el error del modelo.
int test() {
  CHECK_GT(FLAGS_model.size(), 0) << "Hace falta un modelo para testear.";
  CHECK_GT(FLAGS_weights.size(), 0) << "Hace falta los parámetros.";
  vector<string> stages;// = get_stages_from_flags();

  LOG(INFO) << "Usa CPU.";
  Caffe::set_mode(Caffe::CPU);

  Net<float> caffe_net(FLAGS_model, caffe::TEST, 0, &stages);
  caffe_net.CopyTrainedLayersFrom(FLAGS_weights);
  LOG(INFO) << "Ejecutando " << FLAGS_iterations << " iteraciones.";

  vector<int> test_score_output_id;
  vector<float> test_score;
  float loss = 0;
  for (int i = 0; i < FLAGS_iterations; ++i) {
    float iter_loss;
    const vector<Blob<float>*>& result =
        caffe_net.Forward(&iter_loss);
    loss += iter_loss;
    int idx = 0;
    for (int j = 0; j < result.size(); ++j) {
      const float* result_vec = result[j]->cpu_data();
      for (int k = 0; k < result[j]->count(); ++k, ++idx) {
        const float score = result_vec[k];
        if (i == 0) {
          test_score.push_back(score);
          test_score_output_id.push_back(j);
        } else {
          test_score[idx] += score;
        }
        const std::string& output_name = caffe_net.blob_names()[
            caffe_net.output_blob_indices()[j]];
        LOG(INFO) << "Batch " << i << ", " << output_name << " = " << score;
      }
    }
  }
  loss /= FLAGS_iterations;
  LOG(INFO) << "Loss: " << loss;
  for (int i = 0; i < test_score.size(); ++i) {
    const std::string& output_name = caffe_net.blob_names()[
        caffe_net.output_blob_indices()[test_score_output_id[i]]];
    const float loss_weight = caffe_net.blob_loss_weights()[
        caffe_net.output_blob_indices()[test_score_output_id[i]]];
    std::ostringstream loss_msg_stream;
    const float mean_score = test_score[i] / FLAGS_iterations;
    if (loss_weight) {
      loss_msg_stream << " (* " << loss_weight
                      << " = " << loss_weight * mean_score << " loss)";
    }
    LOG(INFO) << output_name << " = " << mean_score << loss_msg_stream.str();
  }

  return 0;
}
RegisterBrewFunction(test);


// Benchmarking
int time() {
  CHECK_GT(FLAGS_model.size(), 0) << "Se necesita la definición de un modelo para hacer benchmarking.";
  caffe::Phase phase = get_phase_from_flags(caffe::TRAIN);
  vector<string> stages; //= get_stages_from_flags();

  LOG(INFO) << "Use CPU.";
  Caffe::set_mode(Caffe::CPU);

  Net<float> caffe_net(FLAGS_model, phase, 0, &stages);

  LOG(INFO) << "Realizando Forward";

  float initial_loss;
  caffe_net.Forward(&initial_loss);
  LOG(INFO) << "Initial loss: " << initial_loss;
  LOG(INFO) << "Realizando Backward";
  caffe_net.Backward();

  const vector<shared_ptr<Layer<float> > >& layers = caffe_net.layers();
  const vector<vector<Blob<float>*> >& bottom_vecs = caffe_net.bottom_vecs();
  const vector<vector<Blob<float>*> >& top_vecs = caffe_net.top_vecs();
  //const vector<vector<bool> >& bottom_need_backward = caffe_net.bottom_need_backward();
  LOG(INFO) << "*** Inicio Benchmarking ***";
  LOG(INFO) << "En cada iteracion se ejecuta Forward y Backward";
  LOG(INFO) << "Probando " << FLAGS_iterations << " iteraciones.";
  Timer total_timer;
  total_timer.Start();
  Timer forward_timer;
  //Timer backward_timer;
  Timer timer;
  std::vector<double> forward_time_per_layer(layers.size(), 0.0);
  //std::vector<double> backward_time_per_layer(layers.size(), 0.0);
  double forward_time = 0.0;
  //double backward_time = 0.0;
  for (int j = 0; j < FLAGS_iterations; ++j) {
    Timer iter_timer;
    iter_timer.Start();
    forward_timer.Start();
    for (int i = 0; i < layers.size(); ++i) {
      timer.Start();
      layers[i]->Forward(bottom_vecs[i], top_vecs[i]);
      forward_time_per_layer[i] += timer.MicroSeconds();
    }
    forward_time += forward_timer.MicroSeconds();
    
    /*
    backward_timer.Start();
    for (int i = layers.size() - 1; i >= 0; --i) {
      timer.Start();
      layers[i]->Backward(top_vecs[i], bottom_need_backward[i],
                          bottom_vecs[i]);
      backward_time_per_layer[i] += timer.MicroSeconds();
    }
    backward_time += backward_timer.MicroSeconds();
    */
    LOG(INFO) << "Iteration: " << j + 1 << " forward-backward time: "
      << iter_timer.MilliSeconds() << " ms.";
    }
  LOG(INFO) << "Promedio time per layer: ";
  for (int i = 0; i < layers.size(); ++i) {
    const caffe::string& layername = layers[i]->layer_param().name();
    LOG(INFO) << std::setfill(' ') << std::setw(10) << layername <<
      "\tforward: " << forward_time_per_layer[i] / 1000 /
      FLAGS_iterations << " ms.";
    //LOG(INFO) << std::setfill(' ') << std::setw(10) << layername  <<
    //  "\tbackward: " << backward_time_per_layer[i] / 1000 /
    //  FLAGS_iterations << " ms.";
  }
  total_timer.Stop();
  LOG(INFO) << "Promedio Forward pass: " << forward_time / 1000 /
    FLAGS_iterations << " ms.";
  LOG(INFO) << "Total: " << total_timer.MilliSeconds() << " ms.";
  LOG(INFO) << "*** Fin Benchmarking ***";
  return 0;
}
RegisterBrewFunction(time);

int main(int argc, char** argv) {

  FLAGS_alsologtostderr = 1;
  
  gflags::SetVersionString(AS_STRING(CAFFE_VERSION));
  
  gflags::SetUsageMessage("command line brew\n"
      "uso: clasificador <command> <args>\n\n"
      "camandos:\n"
      "  train           entrenar o continuar entrenado una red\n"
      "  time            generar tiempos del modelo");
  
  caffe::GlobalInit(&argc, &argv);
  
  if (argc == 2) {
      return GetBrewFunction(caffe::string(argv[1]))();
  } else {
    gflags::ShowUsageWithFlagsRestrict(argv[0], "project/herramientas/entrenador");
  }
}
