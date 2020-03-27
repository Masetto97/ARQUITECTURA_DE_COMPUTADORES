#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

//Trabajo realizado por Jorge Muñoz Martinez y Eduardo Mora González

#define COLOUR_DEPTH 4
//definimos una matriz de 256 posiciones que usaremos en los metodos Manual
int matriz[256];
//definimos una matriz que tendra para cada metodo la cuenta de los bits grises 
int histograma[256];
//definimos una matriz que tendra el numero de bits grises del metodo secuencial
int histogramaAux[256];
//creamos los cerrojos que se usaran en el metodo lock
omp_lock_t lock[256];

//en este metodo comparamos la matriz auxiliar (tiene los nº de bits del secuencial) 
//para cada histograma 
void comparar(){
	bool salida=true;
	for(int i=0;i<256;i++){
		if(histograma[i]!=histogramaAux[i]){
			salida=false;
		}
	}
	if(salida==true){
		printf("histograma correcto\n");
	}else{
		printf("histograma incorrecto\n");
	}
//si algunos de los valores del histograma es distinto del histograma aux la variable booleana 
//cambia a false e imprime histograma incorrecto, en caso de que sean iguales imprime histograma correcto
}

//en este metodo se ponen todos loa valores de la matriz histograma a 0
void borrarHistograma(){
	for(int i=0;i<256;i++){
		histograma[i]=0;
	}
}

//en este metodo se ponen los valores de la matriz a 0
void borrarMatriz(){
	for(int i=0;i<256;i++){
		matriz[i]=0;
	}
}

//metodo critical
double computeGrayParallel(QImage *image) {
	printf("\n");
	printf("critical\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
   
#pragma omp parallel for   
 
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
    
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
	#pragma opm critical
	{
    	histograma[gray]++;
  	}
  }
//comparamos el hitograma con el histogramaAux 
  comparar();
//borramos el histograma para poder usarlo mas tarde 
  borrarHistograma();
  return omp_get_wtime() - start_time;  
}

//metodo atomic
double computeGrayAtomic(QImage *image){
	printf("\n");
	printf("atomic\n");
	double start_time=omp_get_wtime();
	uchar *pixelPtr=image->bits();

#pragma omp parallel for
	for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
   
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii); 
    int gray = qGray(*rgbpixel);
    #pragma omp atomic
    histograma[gray]++;
  }
//comparamos el hitograma con el histogramaAux 
  comparar();
  borrarHistograma();
//borramos el histograma para poder usarlo mas tarde 
  return omp_get_wtime() - start_time;  
}

//secuencial
double computeGraySequential(QImage *image) {
	printf("\n");
	printf("Secuencial\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  //recorremos la imagen
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
   
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
    histogramaAux[gray]++;
  }
  return omp_get_wtime() - start_time;  
}

//creación de los 256 locks
void createLocks(){
	for(int i=0;i<256;i++){
		omp_init_lock(&lock[i]);
	}
}

//eliminación de los 256 locks
void destroyLocks(){
	for(int i=0;i<256;i++){
		omp_destroy_lock(&lock[i]);
	}
}

//metodo locks
double computeGrayLock(QImage *image) {
	printf("\n");
printf("Lock\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
//creamos los locks
  createLocks();
  for (int ii = 0; ii < image->byteCount(); ii += COLOUR_DEPTH) {
   
    QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    int gray = qGray(*rgbpixel);
	//para cada lock bloquemaos el lock i
    omp_set_lock(&lock[gray]);
    histograma[gray]++;
	//para cada lock desbloquemos el lock i
    omp_unset_lock(&lock[gray]);
  }
//eliminamos los lock creados 
  destroyLocks();
//comparamos el histograma con el histogramaAux
  comparar();
//borrar histograma para poder usarlo luego 
  borrarHistograma();
  return omp_get_wtime() - start_time;  
}

//metodo manual 
double computeGrayManual(QImage *image) {
	printf("\n");
	printf("Manual\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  int hebras=omp_get_num_threads();
  //recorremos el número de hebras (en este caso el número de núcleos con el omp_get_num_threads())
 	for(int i=0;i<hebras;i++){
 		borrarMatriz();
		//usaremos una matriz donde almacenamos los grises para cada hebra
		//despues de cada iteración ponemos la matriz a 0 todas las posiciones
 		long inicio=((image->byteCount()*i)/hebras);
 		long fin=((image->byteCount()*(i+1))/hebras);
		//definimos las funcioes de inicio y final 
  		for (int ii = inicio; ii < fin; ii += COLOUR_DEPTH) {
  			QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    		int gray = qGray(*rgbpixel);
    		matriz[gray]++;
    		}
		//despues de recorrer la zona entre el inicio y el fin sumamos el contenido
		//de la matriz auxiliar con el histograma 
    		for(int j=0;j<256;j++){
    			histograma[j]+=matriz[j];
  		}
 	}
//comparamos el histograma con el histoigramaAux
  comparar();
//borramos el histograma
  borrarHistograma();
return omp_get_wtime() - start_time; 
}

//metodo manual con 5 hebras
double computeGrayManual5Hebras(QImage *image) {

	printf("\n");
	printf("Manual 5 hebras\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  int hebras=5;
  //igual que el metodo anterior pero el número de hebras es 5 no el nº de nodos 
 	for(int i=0;i<hebras;i++){
 		borrarMatriz();
 		long inicio=((image->byteCount()*i)/hebras);
 		long fin=((image->byteCount()*(i+1))/hebras);
  		for (int ii = inicio; ii < fin; ii += COLOUR_DEPTH) {
  			QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    		int gray = qGray(*rgbpixel);
    		matriz[gray]++;
    		}
    		for(int j=0;j<256;j++){
    			histograma[j]+=matriz[j];
  		}
 	}
  comparar();
  borrarHistograma();
return omp_get_wtime() - start_time; 
}

//metodo manual con 1 hebras
double computeGrayManual1Hebra(QImage *image) {

	printf("\n");
	printf("Manual 1 hebra\n");
  double start_time = omp_get_wtime();
  uchar *pixelPtr = image->bits();
  int hebras=1;
  //igual que el metodo anterior pero el número de hebras es 1 no el nº de nodos 
 	for(int i=0;i<hebras;i++){
 		borrarMatriz();
 		long inicio=((image->byteCount()*i)/hebras);
 		long fin=((image->byteCount()*(i+1))/hebras);
  		for (int ii = inicio; ii < fin; ii += COLOUR_DEPTH) {
  			QRgb* rgbpixel = reinterpret_cast<QRgb*>(pixelPtr + ii);
    		int gray = qGray(*rgbpixel);
    		matriz[gray]++;
    		}
    		for(int j=0;j<256;j++){
    			histograma[j]+=matriz[j];
  		}
 	}
  comparar();
  borrarHistograma();
return omp_get_wtime() - start_time; 
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_1080p.bmp"); // ("c:\\test_1080p.bmp");
    if(qp.isNull())
    {
        printf("image not found\n");
	return -1;
    }
    
    QImage image = qp.toImage();
    
	//mostramos los tiempos de ejecución de cada metodo
    double computeTime = computeGraySequential(&image);
    printf("sequential time: %0.9f seconds\n", computeTime);
    
    computeTime = computeGrayParallel(&image);
    printf("parallel time: %0.9f seconds\n", computeTime);

    computeTime=computeGrayAtomic(&image);
    printf("atomic time: %0.9f seconds\n", computeTime);
   
	computeTime=computeGrayLock(&image);
    printf("lock time: %0.9f seconds\n", computeTime);

    computeTime=computeGrayManual(&image);
    printf("manual time: %0.9f seconds\n", computeTime);

	computeTime=computeGrayManual5Hebras(&image);
    printf("manual time 5 hebras: %0.9f seconds\n", computeTime);

    computeTime=computeGrayManual1Hebra(&image);
    printf("manual time 1 hebras: %0.9f seconds\n", computeTime);
  

}


//Conclusiones de los resultados:

//Tras ejecutar varias veces las diferentes formas de solucionar el programa el que mas tarda es el Critical, luego el manual y luego el de 5 hebras...
//pero estos resultados pueden cambiar, ya que hemos ejecutado el mismo codigo varias veces y en distincas computadoras y el Critical se cambia con el 
//manual. 

//Antes de realizar las pruebas y comparar los resultados hemos pensado que el mejor deberia haber sido el de 5 hebras ya que pensabamos que al dibidir la
//ejecución en 5 hebras a la vez, este tardaria menos y el que más el secuencial.

//Pero como hemos mencionado antes, los pensamientos iniciales han cambiado


