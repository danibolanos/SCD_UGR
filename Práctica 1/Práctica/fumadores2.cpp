#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
//
// Alumno: Daniel Bolaños Martínez.
//
// fumadores2.cpp
//
// Examen Práctica 1
//----------------------------------------------------------------------

// variables compartidas

Semaphore mostr_vacio=1;
Semaphore ingr_disp[4]={0,0,0,0};	//Declaro 4 semáforos uno para cada ingrediente

int cigarros_fum=0; 	//Total de cigarros fumados. Inicialmente 0.
mutex mux;		//Mutex para evitar exclusión mutua

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//-------------------------------------------------------------------------
// función producir, produce ingrediente aleatorio dado por el estanquero

int producir(){
  // calcular milisegundos aleatorios de duración de la acción de dispensar un ingrediente)
   chrono::milliseconds duracion_dispensar( aleatorio<20,200>() );
   int ingrediente( aleatorio<0,3>() );
  // espera bloqueada un tiempo igual a ''duracion_dispensar' milisegundos
   this_thread::sleep_for( duracion_dispensar );
   return ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   int i;
   while( true ){
      i = producir();
      sem_wait( mostr_vacio );
      cout << "Puesto ingr.: " << i << endl;
      sem_signal( ingr_disp[i] );
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    //Incrementa la variable de cigarros fumados, usa un mutex
    mux.lock();
    cigarros_fum++;
    mux.unlock();
    //Muestra el número de cigarros totales fumados hasta ese momento
    cout << "Cigarros fumados totales: " << cigarros_fum << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   int b = num_fumador;
   while( true )
   {
      sem_wait( ingr_disp[b] );
      //Retira el ingrediente y muestra mensaje de mostrador vacío.
      cout << "Retirado ingr.: " << b << endl;
      cout << "Mostrador vacio." << endl;
      
      //Si el número de cigarros fumados es par primero fuma y luego avisa de que puede poner otro ingrediente
      if(cigarros_fum % 2 == 0){
         fumar( b );
         sem_signal( mostr_vacio );
      }
      //En otro caso (impar) primero avisa de que el mostrador está vacio y luego fuma
      else{
         sem_signal( mostr_vacio );
         fumar( b );
      }
   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
      cout << "--------------------------------------------------------" << endl
        << "Problema de los Fumadores." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_estanquero;
   hebra_estanquero = thread( funcion_hebra_estanquero );
   thread hebra_fumador[4] ;
   for( int i = 0 ; i < 4 ; i++ )
      hebra_fumador[i] = thread( funcion_hebra_fumador, i ) ;

   hebra_estanquero.join();
   for( int i = 0 ; i < 4 ; i++ )
      hebra_fumador[i].join();

}
