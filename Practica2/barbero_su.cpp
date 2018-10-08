// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// archivo: barbero_su.cpp
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

const int N = 5;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void CortarPeloACliente(){
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
   cout << "Cliente pelado en (" << duracion_pelar.count() << " milisegundos)" << endl;
   this_thread::sleep_for( duracion_pelar );
}

void EsperarFueraBarberia( int i ){
   chrono::milliseconds duracion_esperar( aleatorio<20,200>() );
   cout << "Cliente " << i << " espera fuera durante (" << duracion_esperar.count() << " milisegundos)" << endl;
   this_thread::sleep_for( duracion_esperar );
}

// *****************************************************************************
// clase para monitor Barberia,  semántica SU

class Barberia : public HoareMonitor
{
   private:
   CondVar cond_clientes,
           cond_silla,
           cond_barbero;

   public:
   Barberia( ) ; // constructor
   void cortarPelo( int i );
   void siguienteCliente( );
   void finCliente( );
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia(  )
{
   cond_clientes = newCondVar();
   cond_barbero = newCondVar();
   cond_silla = newCondVar();
}
// -----------------------------------------------------------------------------

void Barberia::cortarPelo( int i )
{
   cout << "Cliente numero " << i << " entra a la Barberia y espera su turno." << endl;
   if(cond_clientes.get_nwt() != 0 || cond_silla.get_nwt() !=0)
      cond_clientes.wait();


   cout << "Cliente numero " << i << " se dispone a pelarse." << endl;

   cond_barbero.signal();
   cond_silla.wait();
}

void Barberia::siguienteCliente( )
{
   if(cond_silla.get_nwt() ==0){
      cout << "Barbero se duerme..." << endl;
      cond_barbero.wait();  
   }
}

void Barberia::finCliente( )
{
   cout << "Barbero despide al Cliente." << endl;
   cond_silla.signal();
   cond_clientes.signal();
}

// -----------------------------------------------------------------------------

void  funcion_hebra_cliente( MRef<Barberia> monitor, int num_cliente )
{
   while( true )
   {
      monitor->cortarPelo( num_cliente );
      EsperarFueraBarberia( num_cliente );
   }
}

void funcion_hebra_barbero( MRef<Barberia> monitor )
{
   while( true ){
      monitor->siguienteCliente();
      CortarPeloACliente();
      monitor->finCliente();
   }
}

// *****************************************************************************

int main()
{
   // crear monitor
   auto monitor = Create<Barberia>( );

   // crear y lanzar hebras
   thread hebra_clientes[N], hebra_barbero;
   for( unsigned i = 0 ; i < N ; i++ )
   {
      hebra_clientes[i] = thread( funcion_hebra_cliente, monitor, i );
   }

   hebra_barbero = thread( funcion_hebra_barbero, monitor );

   // esperar a que terminen las hebras (no pasa nunca)
   for( unsigned i = 0 ; i < N ; i++ )
   {
      hebra_clientes[i].join();
   }
   hebra_barbero.join();
}
