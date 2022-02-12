/**
 * Pedro Manuel Flores Crespo
 * Sistemas Concurrentes y distribuidos
 * Solución sl problema de los fumadores
 */


#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <vector>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas
Semaphore mostr_vacio=1;     //Semáforo para saber si el mostrador está vacío
Semaphore todos_terminados[3] ={0,0,0};     //Avisar que todos han terminado de fumar
Semaphore ingr_disp[3]={0,0,0};   //Semáforo para cada uno de los ingredientes

const int N = 5;
int j;

/** Compilar
 * g++ -std=c++11 -o ejecutable fumadores-finito.cpp -I. Semaphore.cpp -lpthread
 */

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

//-------------------------------------------------------------------------
// función producir, produce ingrediente aleatorio dado por el estanquero

int producir(){
  // calcular milisegundos aleatorios de duración de la acción de dispensar un ingrediente)
   chrono::milliseconds duracion_dispensar( aleatorio<20,200>() );
   int ingrediente( aleatorio<0,2>() );
  // espera bloqueada un tiempo igual a ''duracion_dispensar' milisegundos
   this_thread::sleep_for( duracion_dispensar );
   return ingrediente;
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  int i;
  for(j=0; j<N; ++j){
    i = producir();     		//Producimos el ingrediente
    sem_wait(mostr_vacio);              //Vemos si el mostrador está vacío
    cout << "Puesto ingr.: " << i << endl;      //Ponemos el ingrediente
    sem_signal(ingr_disp[i]);    		//Avisamos al cliente de dicho ingrediente
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

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   bool despertado = true;
   int b = num_fumador;
   while( j < N || despertado)
   {
        sem_wait(ingr_disp[num_fumador]);   //Esperamos a que está nuestro producto
        if(despertado){
          cout << "Retirado ingr.: " << b << endl;   //Lo quitamos
          sem_signal(mostr_vacio);     //Le decimos al estanquero que el mostrador está vacío
          fumar(b);   //Fumamos
        }

        if(j == N && despertado){
          despertado = false;
          for(int i=0; i<3; ++i){
            if(i != b){
              sem_signal(ingr_disp[i]);
            }
          }
        }
   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
  << "Problema de los fumadores." << endl
  << "--------------------------------------------------------" << endl
  << flush ;

  thread hebra_estanquero ( funcion_hebra_estanquero );   //Lanzamos la hebra estanquero

  thread hebra_fumadores[3];    //Lanzamos las hebras clientes
  for(int i=0; i<3; ++i){
    hebra_fumadores[i] = thread(funcion_hebra_fumador, i);
  }

  hebra_estanquero.join();   //Las unimos
  for(int i=0; i<3; ++i){
    hebra_fumadores[i].join();
  }
}
