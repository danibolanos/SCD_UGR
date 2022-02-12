#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
//
// Alumno: Daniel Bolaños Martínez
// Alternativa FIFO (First In First Out)

// variables compartidas

const int num_items = 60 ,   // número de items
	  tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

int buffer[tam_vec];		//vector que servirá de buffer
int primera_libre = 0,	  /*índice en el vector de la primera celda libre 
                          Esta variable se incrementa al escribir (módulo tam_vec)*/
    primera_ocupada = 0;  /*índice en el vector de la primera celda ocupada 
                          Esta variable se incrementa al leer (módulo tam_vec)*/

//Semáforos
//Inicializados ocupados a 0 y libres a 1

Semaphore ocupadas=0, libres=tam_vec;

//Mutex para evitar exclusión mutua
mutex prod, cons;

const int PRODUCTORES=10, CONSUMIDORES=6;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{ 
/* Utilizaremos este bucle para producir el buffer 
en cada iteración siguiendo la filosofía FIFO*/

for( unsigned i = 0 ; i < num_items/PRODUCTORES ; i++ )
{
   sem_wait(libres);     // Libres comienza en tam_vec por lo que productor 
                         // no espera inicialmente, se va decrementando tam_vec veces
      
   int dato = producir_dato();
   prod.lock();
   buffer[primera_libre] = dato;

   sem_signal(ocupadas);  // Se manda señal al semáforo ocupadas
			  // para que se pueda consumir	
   
   primera_libre = (primera_libre+1) % tam_vec;  // Incremento modulo tam_vec (FIFO)
   prod.unlock();
}
cout << "\nFin hebra productora\n\n";  // Fin hebra productora
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
/*Utilizaremos este bucle para consumir el buffer 
en cada iteración siguiendo la filosofía FIFO*/

for( int i=0; i < num_items/CONSUMIDORES; i++)
{
   sem_wait(ocupadas); // La hebra consumidora no puede consumir inicialmente 
		       // hasta que la productora produzca los datos
		       // del buffer intermedio
   cons.lock();
   int dato = buffer[primera_ocupada];
   consumir_dato(dato);

   sem_signal(libres);  // Se manda señal al semáforo libres
			// para que se pueda producir
       
   primera_ocupada = (primera_ocupada+1) % tam_vec; //Incremento modulo tam_vec (FIFO)
   cons.unlock();
}
cout << "\nFin hebra consumidora\n\n";  // Fin hebra consumidora
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_consumidor[CONSUMIDORES];
   for( int i = 0 ; i < CONSUMIDORES ; i++ )
      hebra_consumidor[i] = thread( funcion_hebra_consumidora ) ;

   thread hebra_productor[PRODUCTORES] ;
   for( int i = 0 ; i < PRODUCTORES ; i++ )
      hebra_productor[i] = thread( funcion_hebra_productora ) ;

   for( int i = 0 ; i < CONSUMIDORES ; i++ )
      hebra_consumidor[i].join() ;

   for( int i = 0 ; i < PRODUCTORES ; i++ )
      hebra_productor[i].join() ;

   test_contadores();
}
