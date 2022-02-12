// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// archivo: fumadores_su.cpp
//
// Alumno: Daniel Bolaños Martínez
//
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <random>
#include <mutex>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

mutex mux;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int producir(){
  // calcular milisegundos aleatorios de duración de la acción de dispensar un ingrediente)
   chrono::milliseconds duracion_dispensar( aleatorio<20,200>() );
   int ingrediente( aleatorio<0,2>() );
  // espera bloqueada un tiempo igual a ''duracion_dispensar' milisegundos
   this_thread::sleep_for( duracion_dispensar );
   return ingrediente;
}

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


// *****************************************************************************
// clase para monitor Estanquero,  semántica SU

class Estanco : public HoareMonitor
{
   private:
   int      mostrador;             // mostrador vacío
   CondVar  Cestanquero,           // cola del estanquero
	    Cfumador[3];	   // cola de los fumadores

   public:
   Estanco( ) ; // constructor
   void poner_ingrediente( int ing );
   void esperar_recogida( );
   void obtener_ingrediente( int ing );
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco(  )
{
   mostrador = -1 ;
   Cestanquero            = newCondVar();
   for(int i=0; i<3; i++)
      Cfumador[i]            = newCondVar();
}
// -----------------------------------------------------------------------------

void Estanco::poner_ingrediente( int ing )
{
   mostrador = ing;
   cout << "Puesto ingr: " << ing << endl;
   Cfumador[ing].signal();
}

void Estanco::esperar_recogida( )
{
   if(mostrador!=-1)
      Cestanquero.wait();

}

void Estanco::obtener_ingrediente( int ing )
{
   if(mostrador!=ing)
      Cfumador[ing].wait();
   mostrador=-1;
   cout << "Retirado ing: " << ing << endl;
   Cestanquero.signal();
}

// -----------------------------------------------------------------------------

void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      monitor->obtener_ingrediente( num_fumador );
      fumar( num_fumador );
   }
}

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
   int ing;
   while( true ){
      ing = producir();
      monitor->poner_ingrediente(ing);
      monitor->esperar_recogida();
   }
}

// *****************************************************************************

int main()
{
   // crear monitor
   auto monitor = Create<Estanco>( );

   // crear y lanzar hebras
   thread hebra_fumador[3], hebra_estanquero;
   for( unsigned i = 0 ; i < 3 ; i++ )
   {
      hebra_fumador[i] = thread( funcion_hebra_fumador, monitor, i );
   }

   hebra_estanquero = thread( funcion_hebra_estanquero, monitor );

   // esperar a que terminen las hebras (no pasa nunca)
   for( unsigned i = 0 ; i < 3 ; i++ )
   {
      hebra_fumador[i].join();
   }
   hebra_estanquero.join();
}
