#include <QtGui/QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <omp.h>

#define COLOUR_DEPTH 4

double medianFilter(QImage* image, QImage* result) {
  double start_time = omp_get_wtime();
  
  QRgb* pixelPtr = (QRgb*) image->bits();
  QRgb* resultPtr = (QRgb*) result->bits();
  
  
  for (int h = 1; h < image->height() - 1; h++)
    
    for (int w = 1; w < image->width() - 1; w++) {

      QRgb window[9];	// para guardar los 9 píxeles en torno al pixel de coordenadas (h,w)
	  int k = 0;
      for (int y = -1; y < 2; y++)	
		for (int x = -1; x < 2; x++)
			window[k++] = pixelPtr[(h + y) * image->width() + (w + x)];
			/* El pixel de coordenadas (a,b) ocupa en memoria la posición, 
			a*image->width()+b (relativa al inicio de la imagen), ya que antes de ese pixel hay:
			a filas de píxeles completas más b pixeles en la propia fila */
      
      //   Ordenar los 5 primeros elementos para obtener la mediana
      for (int j = 0; j < 5; ++j) {
		//   Encontrar el elemento que ocuparía la posición j en la lista ordenada
		int min = j;
		for (int l = j + 1; l < 9; ++l)
			if (window[l] < window[min]) min = l;
		//   Poner el elmento encontrado en la posición j de window
		QRgb temp = window[j];
		window[j] = window[min];
		window[min] = temp;
      }

      //   La mediana es window[4] (el 5.º de los 9 elementos)
      resultPtr[h * image->width() + w] = window[4];
    }
  
  return omp_get_wtime() - start_time;    
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QPixmap qp = QPixmap("test_ruido.bmp");
    if(qp.isNull())
    {
        printf("image not found\n");
	return -1;
    }
    
    QImage image = qp.toImage();
    QImage result1(image);
    
    double computeTime = medianFilter(&image, &result1);
    printf("sequential time: %0.9f seconds\n", computeTime);

    QPixmap pixmap = pixmap.fromImage(result1);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    scene.addItem(item);

    view.show();
    return a.exec();
}

