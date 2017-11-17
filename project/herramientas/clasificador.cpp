#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <fstream>


using namespace caffe;
using std::string;

std::vector<string> etiquetas_;

typedef std::pair<string, float> Prediccion;

class Clasificador {
 public:
  Clasificador(const string& modelo,
             const string& parametros,
             const string& media,
             const string& etiquetas);

  std::vector<Prediccion> Clasificar(const cv::Mat& imamen, int N = 5);

 private:
  void SetMedia(const string& media);

  std::vector<float> Predecir(const cv::Mat& imagen);

  void ArreglarInputLayer(std::vector<cv::Mat>* canales_);

  void Preprocesar(const cv::Mat& imagen,
                  std::vector<cv::Mat>* canales);

 private:
  shared_ptr<Net<float> > red_;
  cv::Size geometria_;
  int num_canales_;
  cv::Mat media_;
  std::vector<string> etiquetas_;
};

Clasificador::Clasificador(const string& modelo,
                       const string& parametros,
                       const string& media,
                       const string& etiquetas) {
#ifdef CPU_ONLY
  Caffe::set_mode(Caffe::CPU);
#else
  Caffe::set_mode(Caffe::GPU);
#endif

  /* Se carga el modelo de la red neuronal y los parámetros. */
  red_.reset(new Net<float>(modelo, TEST));
  red_->CopyTrainedLayersFrom(parametros);

  CHECK_EQ(red_->num_inputs(), 1) << "La red debería tener una sola capa de entrada.";
  CHECK_EQ(red_->num_outputs(), 1) << "La red debería tener una sola capa de salida.";

  Blob<float>* capa_entrada = red_->input_blobs()[0];
  num_canales_ = capa_entrada->channels();
  CHECK(num_canales_ == 3 || num_canales_ == 1)
    << "La capa de entrada debería tner uno (escala de grises) o tres canales (RGB).";
  geometria_ = cv::Size(capa_entrada->width(), capa_entrada->height());

  /* Se carga el archivo con los valores medios. */
  SetMedia(media);

  /* Se cargan las etiquetas, es decir, los nombres de las categorías de clasificación. */
  std::ifstream archivo_etiquetas(etiquetas.c_str());
  CHECK(archivo_etiquetas) << "No se puede abrir el archivo con las etiquetas." << etiquetas;
  string line;
  while (std::getline(archivo_etiquetas, line))
    etiquetas_.push_back(string(line));

  Blob<float>* output_layer = red_->output_blobs()[0];
  CHECK_EQ(etiquetas_.size(), output_layer->channels())
    << "El número de etiquetas es distinto a la dimensión de la capa de salida.";
}

static bool PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs) {
  return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
static std::vector<int> Argmax(const std::vector<float>& v, int N) {
  std::vector<std::pair<float, int> > pairs;
  for (size_t i = 0; i < v.size(); ++i)
    pairs.push_back(std::make_pair(v[i], i));
  std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

  std::vector<int> result;
  for (int i = 0; i < N; ++i)
    result.push_back(pairs[i].second);
  return result;
}

/* Devuelve las N primeras predicciones. */
std::vector<Prediccion> Clasificador::Clasificar(const cv::Mat& img, int N) {
  std::vector<float> salida = Predecir(img);

  N = std::min<int>(etiquetas_.size(), N);
  std::vector<int> maxN = Argmax(salida, N);
  std::vector<Prediccion> Predicciones;
  for (int i = 0; i < N; ++i) {
    int idx = maxN[i];
    Predicciones.push_back(std::make_pair(etiquetas_[idx], salida[idx]));
  }

  return Predicciones;
}

/* Load the mean archivo in binaryproto format. */
void Clasificador::SetMedia(const string& media) {
  BlobProto blob_proto;
  ReadProtoFromBinaryFileOrDie(media.c_str(), &blob_proto);

  /* Convert from BlobProto to Blob<float> */
  Blob<float> media_blob;
  media_blob.FromProto(blob_proto);
  CHECK_EQ(media_blob.channels(), num_canales_)
    << "Number of channels of mean archivo doesn't match input layer.";

  /* El formato del archivo es 32-bit RGB o escala de grises */
  std::vector<cv::Mat> canales;
  float* data = media_blob.mutable_cpu_data();
  for (int i = 0; i < num_canales_; ++i) {
    /* Obtener cada canal individualmente. */
    cv::Mat canal(media_blob.height(), media_blob.width(), CV_32FC1, data);
    canales.push_back(canal);
    data += media_blob.height() * media_blob.width();
  }

  /* Unificar los canales en un solo canal. */
  cv::Mat union_media_canales;
  cv::merge(canales, union_media_canales);

  /* Calcular la media global de los pixeles y crear una imagen con ese valor en cada pixel. */
  cv::Scalar media_global = cv::mean(union_media_canales);
  media_ = cv::Mat(geometria_, union_media_canales.type(), media_global);
}

std::vector<float> Clasificador::Predecir(const cv::Mat& imagen) {
  Blob<float>* capa_entrada = red_->input_blobs()[0];
  capa_entrada->Reshape(1, num_canales_,
                       geometria_.height, geometria_.width);
  /* Forward dimension change to all layers. */
  red_->Reshape();

  std::vector<cv::Mat> canales_;
  ArreglarInputLayer(&canales_);

  Preprocesar(imagen, &canales_);

  red_->Forward();

  /* Copiar la salida de la última capa a un vector */
  Blob<float>* capa_salida = red_->output_blobs()[0];
  const float* begin = capa_salida->cpu_data();
  const float* end = begin + capa_salida->channels();
  return std::vector<float>(begin, end);
}

/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last Preprocesaring
 * operation will write the separate channels directly to the input
 * layer. */
void Clasificador::ArreglarInputLayer(std::vector<cv::Mat>* canales_) {
  Blob<float>* capa_entrada = red_->input_blobs()[0];

  int width = capa_entrada->width();
  int height = capa_entrada->height();
  float* input_data = capa_entrada->mutable_cpu_data();
  for (int i = 0; i < capa_entrada->channels(); ++i) {
    cv::Mat channel(height, width, CV_32FC1, input_data);
    canales_->push_back(channel);
    input_data += width * height;
  }
}

/* Este método debe convertir la imágen al formato usado durante el entrenamiento */
void Clasificador::Preprocesar(const cv::Mat& img,
                            std::vector<cv::Mat>* canales_) {
  cv::Mat sample;
  if (img.channels() == 3 && num_canales_ == 1)
    cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
  else if (img.channels() == 4 && num_canales_ == 1)
    cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
  else if (img.channels() == 4 && num_canales_ == 3)
    cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
  else if (img.channels() == 1 && num_canales_ == 3)
    cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
  else
    sample = img;

  cv::Mat sample_resized;
  if (sample.size() != geometria_)
    cv::resize(sample, sample_resized, geometria_);
  else
    sample_resized = sample;

  cv::Mat sample_float;
  if (num_canales_ == 3)
    sample_resized.convertTo(sample_float, CV_32FC3);  
  else
    sample_resized.convertTo(sample_float, CV_32FC1);

  cv::Mat sample_normalized;
  cv::subtract(sample_float, media_, sample_normalized);

  /* This operation will write the separate BGR planes directly to the
   * input layer of the network because it is wrapped by the cv::Mat
   * objects in canales_. */
  cv::split(sample_normalized, *canales_);

  CHECK(reinterpret_cast<float*>(canales_->at(0).data)
        == red_->input_blobs()[0]->cpu_data())
    << "Input channels are not wrapping the input layer of the network.";
}

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cerr << "Uso para clasificar: " << argv[0]
              << " modelo.prototxt parametros.caffemodel"
              << " media.binaryproto etiquetas.txt img.png" << std::endl;

    std::cerr << "Uso para validar: " << argv[0]
			  << "	Validacion: "
              << " modelo.prototxt parametros.caffemodel"
              << " media.binaryproto etiquetas.txt lista_validacion.txt" << std::endl;              
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  string modelo   = argv[1];
  string parametros = argv[2];
  string media    = argv[3];
  string etiquetas   = argv[4];
  Clasificador Clasificador(modelo, parametros, media, etiquetas);

  string archivo = argv[5];

  // Si el último argumento temina en .txt es porque se trata de validar el modelo contra una lista de imágenes
  if (archivo.find(".txt") != std::string::npos) {

    /* Load labels. */
    std::ifstream archivo_etiquetas(etiquetas.c_str());
    string line;
    while (std::getline(archivo_etiquetas, line))
      etiquetas_.push_back(string(line));

    std::cout << "---------- Validacion para "
              << archivo << " ----------" << std::endl;

    int valids=0;
    int total=0;

    std::ifstream inarchivo(archivo.c_str());
    int tag;
    string img_archivo;
    while (inarchivo >> img_archivo >> tag)
    {

      total++;

      cv::Mat img = cv::imread(img_archivo, -1);
      CHECK(!img.empty()) << "Error al decodificar la imagen " << img_archivo;

      std::vector<Prediccion> Prediccions = Clasificador.Clasificar(img);

      Prediccion p = Prediccions[0];

      string Predecired=etiquetas_[tag];

      if(p.first.compare(Predecired)==0){
        std::cout << "---------- Validacion para "
              << img_archivo << " ---------- Correcta " << std::endl;

        valids++;

      }else{
        std::cout << "---------- Validacion para "
              << img_archivo << " ---------- Incorrecta" << std::endl;

      }
    }

    std::cout << "---------- Total:  " << total 
        << " ---------- Validas:  " << valids 
        <<  "---------- Exactitud: " << std::fixed << std::setprecision(2) << (float) (valids * 100) / total << std::endl;
  }else{
    std::cout << "---------- Prediccion para "
              << archivo << " ----------" << std::endl;

    cv::Mat img = cv::imread(archivo, -1);
    CHECK(!img.empty()) << "No se puede decodificar la imagen " << archivo;
    std::vector<Prediccion> Prediccions = Clasificador.Clasificar(img);

    /* Mostrar las primeras N predicciones. */
    for (size_t i = 0; i < Prediccions.size(); ++i) {
      Prediccion p = Prediccions[i];
      std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
                << p.first << "\"" << std::endl;
    }              
  }
}
