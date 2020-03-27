#include <QtGui/QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>
#include <math.h> 

#define COLOUR_DEPTH 4

//Trabajo realizado por Jorge Muñoz Martinez y Eduardo Mora González

int weight[3][3] = {{ 1,  2,  1 },
		                { 0,  0,  0 },
		                { -1,  -2,  -1 }};
int height[3][3]={{-1,0,1},
                  {-2,0,2},
                  {1,0,1}};

double computeSobelSequential(QImage *srcImage, QImage *dstImage,int kernel[3][3]) {
  printf("Sobel Sequential\n");
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj, blue;
 
  for (ii = 1; ii < srcImage->height() - 1; ii++) {  	// Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
       pixelValue = 0;
      for (int j = -1; j <= 1; j++) {					// Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
	    blue = qBlue(srcImage->pixel(jj+i, ii+j));
             pixelValue += kernel[j + 1][i + 1] * blue;	// En pixelValue se calcula el componente y del gradiente
              //pixelValue += height[j+1][i+1]*blue;
          }
      }

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());	// Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

double computeSobelSequentialOp(QImage *srcImage, QImage *dstImage) {
  printf("Sobel Sequential OPcional\n");
  double start_time = omp_get_wtime();
  int pixelValue, pixelValueX, pixelValueY;
  int ii, jj, blue;
 
  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
       pixelValue = 0;
       pixelValueX = 0;
       pixelValueY = 0;
      for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
      blue = qBlue(srcImage->pixel(jj+i, ii+j));
             pixelValueX += weight[j + 1][i + 1] * blue;
             pixelValueY += height[j + 1][i + 1] * blue; // En pixelValue se calcula el componente y del gradiente
              //pixelValue += height[j+1][i+1]*blue;
          }
      }
      pixelValue = sqrt((pixelValueX * pixelValueX) + (pixelValueY * pixelValueY));
      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

double computeSobelParallelOp(QImage *srcImage, QImage *dstImage) {
  printf("Sobel Parallel OPcional\n");
  double start_time = omp_get_wtime();
  int pixelValue, pixelValueX, pixelValueY;
  int ii, jj, blue;
  int division=srcImage->height()/omp_get_num_procs();
  //tamaño de la imagen entre el numero de procesadores 
  #pragma omp parallel for schedule(static,division) private(pixelValue,pixelValueX,pixelValueY,ii,jj) 
  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
      
       pixelValue = 0;
       pixelValueX = 0;
       pixelValueY = 0;
      for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
      blue = qBlue(srcImage->pixel(jj+i, ii+j));
             pixelValueX += weight[j + 1][i + 1] * blue;
             pixelValueY += height[j + 1][i + 1] * blue; // En pixelValue se calcula el componente y del gradiente
              //pixelValue += height[j+1][i+1]*blue;
          }
      }
      pixelValue = sqrt((pixelValueX * pixelValueX) + (pixelValueY * pixelValueY));
      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
} 
double computeSobelParallel(QImage *srcImage, QImage *dstImage,int kernel[3][3]) {
  printf("Sobel parallel\n");
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj, blue;
  int division=srcImage->height()/omp_get_num_procs();
  //tamaño de la imagen entre el numero de procesadores 
  #pragma omp parallel for schedule(static,division) private(pixelValue,ii,jj) 
  for (ii = 1; ii < srcImage->height() - 1; ii++) {   // Recorremos la imagen, excepto los bordes
    for (jj = 1; jj < srcImage->width() - 1; jj++) {   
       pixelValue = 0;
      for (int j = -1; j <= 1; j++) {         // Recorremos el kernel weight[3][3]
          for (int i = -1; i <= 1; i++) {
              blue = qBlue(srcImage->pixel(jj+i, ii+j));
             pixelValue += kernel[j + 1][i + 1] * blue; // En pixelValue se calcula el componente y del gradiente
              //pixelValue += height[j+1][i+1]*blue;
          }
      }
      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
  
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino
    }
  }
  return omp_get_wtime() - start_time;  
}

double computeSobelSequentialLocalidad(QImage *srcImage, QImage *dstImage,int kernel[3][3]) {
  printf("Sobel secuencial localidad\n");
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj;
  int matrizAux[3][3];
  int matrizDatosMoviles[3];

  for(int i=0;i<3;i++){
    matrizDatosMoviles[i]=0;
      for(int j=0;j<3;j++){
        matrizAux[i][j]=0;
      }
  }

 for (ii = 1; ii < srcImage->height() - 1; ii++) {   //filas
    for(int i=0;i<3;i++){//filas nueva matriz
      for(int j=0;j<(3-1);j++){//2 no hay que leer otra vez leo 1 columna menos
        matrizAux[i][j] = qBlue(srcImage->pixel(j, ii+i-1));
        //almaceno en la matriz aux los colores
      }
    }
    for (jj = 1; jj < srcImage->width() - 1; jj++) {//columnas
       pixelValue = 0;
       //datos nuevos 
      for (int j = -1; j <= 1; j++) {  
        matrizDatosMoviles[j+1]= qBlue(srcImage->pixel(jj+1, ii+j));
        //j+1 empieza a contar en -1 y eso fallo    
      }
      //recorro la matriz aux para sumar los valores en la variable
      for(int i=0;i<3;i++){
        pixelValue+=kernel[i][2]*matrizDatosMoviles[i];
        //3 columna 
        for(int j=0;j<(3-1);j++){//2 columnas ya que la restante la sumo antes 
          pixelValue += kernel[i][j] * matrizAux[i][j];
        }
      }
      //modificamos las filas de la matriz 
      for(int i=0;i<3;i++){
          matrizAux[i][0]=matrizAux[i][1];
          matrizAux[i][1]=matrizAux[i][2];
          matrizAux[i][2]=matrizDatosMoviles[i];
          //el dato se mueve      
      }

      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;
      
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino

   }
  }
  return omp_get_wtime() - start_time;  
}

double computeSobelParallelLocalidad(QImage *srcImage, QImage *dstImage,int kernel[3][3]) {
  printf("Sobel parallel localidad\n");
  double start_time = omp_get_wtime();
  int pixelValue;
  int ii, jj;
  int matrizAux[3][3];
  int matrizDatosMoviles[3];

  for(int i=0;i<3;i++){
    matrizDatosMoviles[i]=0;
      for(int j=0;j<3;j++){
        matrizAux[i][j]=0;
      }
  }
  int division=srcImage->height()/omp_get_num_procs();
  #pragma omp parallel for schedule(static,division) private(pixelValue,ii,jj)  
 for (ii = 1; ii < srcImage->height() - 1; ii++) {   
    for(int i=0;i<3;i++){
      for(int j=0;j<(3-1);j++){
        matrizAux[i][j] = qBlue(srcImage->pixel(j, ii+i-1));
      }
    }
    for (jj = 1; jj < srcImage->width() - 1; jj++) {
       pixelValue = 0;
      for (int j = -1; j <= 1; j++) {  
        matrizDatosMoviles[j+1]= qBlue(srcImage->pixel(jj+1, ii+j));  
      }
      for(int i=0;i<3;i++){
        pixelValue+=kernel[i][2]*matrizDatosMoviles[i];
        for(int j=0;j<(3-1);j++){
          pixelValue += kernel[i][j] * matrizAux[i][j];
        }
      }
      for(int i=0;i<3;i++){
          matrizAux[i][0]=matrizAux[i][1];
          matrizAux[i][1]=matrizAux[i][2];
          matrizAux[i][2]=matrizDatosMoviles[i];    
      }
      int new_value = pixelValue;
      if (new_value > 255) new_value = 255;
      if (new_value < 0) new_value = 0;     
      dstImage->setPixel(jj,ii, QColor(new_value, new_value, new_value).rgba());  // Se actualiza la imagen destino

    }
  }
  return omp_get_wtime() - start_time;  
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_1080p.bmp");
    if(qp.isNull())
    {	printf("image not found\n");
		return -1;
    }
    
    QImage image = qp.toImage();
    QImage sobelImage(image);
    
    printf("kernel = weight \n");
    printf("\n");

    double computeTime = computeSobelSequential(&image, &sobelImage,weight);
    printf("sequential time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelParallel(&image, &sobelImage,weight);
    printf("parallel time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelSequentialLocalidad(&image, &sobelImage,weight);
    printf("sequential time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelParallelLocalidad(&image, &sobelImage,weight);
    printf("parallel time: %0.9f seconds\n", computeTime);
    printf("\n");

    printf("kernel = height \n");
    printf("\n");

    computeTime = computeSobelSequential(&image, &sobelImage,weight);
    printf("sequential time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelParallel(&image, &sobelImage,height);
    printf("parallel time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelSequentialLocalidad(&image, &sobelImage,height);
    printf("sequential time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelParallelLocalidad(&image, &sobelImage,height);
    printf("parallel time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelSequentialOp(&image, &sobelImage);
    printf("sequential time: %0.9f seconds\n", computeTime);
    printf("\n");

    computeTime = computeSobelParallelOp(&image, &sobelImage);
    printf("parallel time: %0.9f seconds\n", computeTime);
    printf("\n");

    QPixmap pixmap = pixmap.fromImage(sobelImage);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);

    view.show();
    return a.exec();
}

//Conclusiones de los resultados:

//comparando las imagenes, nos damos cuenta que sale de mejor calidad cuando se hace con las dos matrices (parte opcional), ya que hemos probado con
//varias imagenes distintas y en algunas se nota mejor que en otras la diferencia entre una matriz y cuando se usa dos.
//Comparanado los tiempos, nos damos cuenta de que la funcion que más tarda es la Sobel Parallel y la que menos tarda es son las de localidad, dando igual
//en que posicion se recorra la imagenes.

