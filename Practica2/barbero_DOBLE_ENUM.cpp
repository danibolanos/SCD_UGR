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

const int CLIENTES = 10;
const int BARBEROS = 2;

mutex mux;

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
   CondVar cond_clientes_par,
			  cond_clientes_impar,
           cond_silla[BARBEROS],
           cond_barbero[BARBEROS];
	int num_entrada;

   public:
   Barberia( ) ; // constructor
   void cortarPelo( int cliente );
   void siguienteCliente( int barbero );
   void finCliente( int barbero );
	bool silla_libre( int num );
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia(  )
{
   cond_clientes_par = newCondVar();
   cond_clientes_impar = newCondVar();
   for(int i=0; i < BARBEROS; i++){
   	cond_barbero[i] = newCondVar();
   	cond_silla[i] = newCondVar();
	}
	num_entrada=0;
}
// -----------------------------------------------------------------------------

bool Barberia::silla_libre( int num ){
	bool cond = false;

	if(cond_silla[num].get_nwt() == 0)
		cond = true;
	
	return cond;
}

// -----------------------------------------------------------------------------

void Barberia::cortarPelo( int cliente )
{
   cout << "Cliente numero " << cliente << " entra a la Barberia y espera su turno." << endl;
	
	mux.lock();
	int ticket = num_entrada++;
	mux.unlock();

	if(ticket % 2 == 0){
		if(cond_clientes_par.get_nwt() != 0 || !silla_libre(0))
			cond_clientes_par.wait();

		cout << "Cliente numero " << cliente << " se dispone a pelarse." << endl;

		cond_barbero[0].signal();
		cond_silla[0].wait();
	}
	else{
		if(cond_clientes_impar.get_nwt() != 0 || !silla_libre(1))
			cond_clientes_impar.wait();

		cout << "Cliente numero " << cliente << " se dispone a pelarse." << endl;

		cond_barbero[1].signal();
		cond_silla[1].wait();
	}		
}

void Barberia::siguienteCliente( int barbero )
{
   if(cond_silla[barbero].get_nwt() == 0){
      cout << "Barbero " << barbero << " se duerme..." << endl;
      cond_barbero[barbero].wait();  
   }
}

void Barberia::finCliente( int barbero )
{
  	cout << "Barbero " << barbero << " despide al Cliente." << endl;
   cond_silla[barbero].signal();
   if(barbero == 0)
   	cond_clientes_par.signal();
	else
		cond_clientes_impar.signal();
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

void funcion_hebra_barbero( MRef<Barberia> monitor, int num_barbero )
{
   while( true ){
      monitor->siguienteCliente( num_barbero );
      CortarPeloACliente();
      monitor->finCliente( num_barbero );
   }
}

// *****************************************************************************

int main()
{
   // crear monitor
   auto monitor = Create<Barberia>( );

   // crear y lanzar hebras
   thread hebra_clientes[CLIENTES], hebra_barbero[BARBEROS];
   for( unsigned i = 0 ; i < CLIENTES ; i++ )
   {
      hebra_clientes[i] = thread( funcion_hebra_cliente, monitor, i );
   }

   for( unsigned i = 0 ; i < BARBEROS ; i++ )
   {
      hebra_barbero[i] = thread( funcion_hebra_barbero, monitor, i );
   }

   // esperar a que terminen las hebras (no pasa nunca)
   for( unsigned i = 0 ; i < CLIENTES ; i++ )
   {
      hebra_clientes[i].join();
   }
   for( unsigned i = 0 ; i < BARBEROS ; i++ )
   {
      hebra_barbero[i].join();
   }

}
