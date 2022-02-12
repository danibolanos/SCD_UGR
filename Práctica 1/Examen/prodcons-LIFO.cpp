#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <future>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
//
// Alumno: Daniel Bolaños Martínez
// Alternativa LIFO (Last In First Out)

// variables compartidas

const int num_items = 60 ,   // número de items
	  tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

int buffer[tam_vec];		//vector que servirá de buffer
int primera_libre = 0; 		//indice en el vector de la primera celda libre
				//se incrementa al escribir y decrementa al leer

//Semáforos
//Inicializados ocupados a 0 y libres a 1

Semaphore ocupadas=0, libres=tam_vec;

//Mutex para evitar exclusión mutua
mutex mux;

const int PRODUCTORES=2, CONSUMIDORES=3;

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
en cada iteración siguiendo la filosofía LIFO*/

for( unsigned i = 0 ; i < num_items/PRODUCTORES ; i++ )
{
   int dato = producir_dato();
   sem_wait(libres);   // Libres comienza en tam_vec por lo que productor 
                       // no espera inicialmente, se va decrementando tam_vec veces
   mux.lock();
   buffer[primera_libre] = dato;
   primera_libre++;	// Se incrementa al leer
   mux.unlock();    

   sem_signal(ocupadas);   // Se manda señal al semáforo ocupadas
			   // para que se pueda consumir
}
cout << "\nFin hebra productora\n\n"; // Fin hebra productora
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
/*Utilizaremos este bucle para consumir el buffer 
en cada iteración siguiendo la filosofía LIFO*/

for( unsigned i = 0 ; i < num_items/CONSUMIDORES ; i++ )
{
   sem_wait(ocupadas);  // La hebra consumidora no puede consumir inicialmente 
			// hasta que la productora produzca los datos 
			// del buffer intermedio      
   
   mux.lock();
   primera_libre--; // Se decrementa al escribir
   int dato = buffer[primera_libre];
   mux.unlock();
   sem_signal(libres);   // Se manda señal al semáforo libres
			 // para que se pueda producir
   consumir_dato(dato);
}
cout << "\nFin hebra consumidora\n\n"; // Fin hebra consumidora
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
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
