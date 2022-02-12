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

const int BARBEROS = 1;
const int CLIENTES = 20;

int contador=0, aux=0;
bool open = true;

mutex mux;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void CortarPeloACliente(){
	aux++;
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
	mux.lock();
   cout << "Cliente pelado en (" << duracion_pelar.count() << " milisegundos)" << endl << flush;
	mux.unlock();
   this_thread::sleep_for( duracion_pelar );
}

void EsperarFueraBarberia( int i ){
   chrono::milliseconds duracion_esperar( aleatorio<20,200>() );
	mux.lock();
   cout << "Cliente " << i << " espera fuera durante (" << duracion_esperar.count() << " milisegundos)" << endl << flush;
	mux.unlock();
   this_thread::sleep_for( duracion_esperar );
}

class Barberia : public HoareMonitor
{
 private:
 CondVar              
   cond_barbero[BARBEROS],               
   cond_clientes,                 
   cond_silla[BARBEROS];      
	const int MAX_PELADOS=10;       
 public:                    
   Barberia();           
   void cortarPelo(int num_cliente);
   void siguienteCliente(int num_barbero);
   void finCliente(int num_barbero);
   bool sillas_libres();
   int siguiente_silla();
} ;

Barberia::Barberia(  )
{
   for(int i=0; i<BARBEROS; i++)
      cond_barbero[i] = newCondVar();
   cond_clientes = newCondVar();
   for(int i=0; i<BARBEROS; i++)
      cond_silla[i] = newCondVar();
}

// -----------------------------------------------------------------------------

bool Barberia::sillas_libres( ){
	bool cond = false;

	for(int i=0; i < BARBEROS; i++){
		if(cond_silla[i].get_nwt() == 0){
				cond = true;
		}
	}
	
	return cond;
}
// -----------------------------------------------------------------------------

int Barberia::siguiente_silla(  ){
   int sig_silla;

   bool para = false;
   for(int i=0; i < BARBEROS && !para; i++)
      if(cond_silla[i].get_nwt() == 0){
         sig_silla = i;
         para = true;
   }

   return sig_silla;
}

void Barberia::cortarPelo(int num_cliente){
   if(open){
   	cout << "Cliente numero " << num_cliente << " entra a la Barberia y espera su turno." << endl;
   if(cond_clientes.get_nwt()!=0 || !sillas_libres()){
      cond_clientes.wait();
   }
   if(open){
   	cout << "Cliente numero " << num_cliente << " se dispone a pelarse." << endl;
		cond_barbero[siguiente_silla()].signal();
		cond_silla[siguiente_silla()].wait();
   }
   }
}

void Barberia::siguienteCliente(int num_barbero){
   
   contador++;
   if(contador<MAX_PELADOS){
   	if (cond_silla[num_barbero].empty()){
			cout << "Barbero se duerme..." << endl;
      	cond_barbero[num_barbero].wait();
		}
	}
	else {
      open=false;
		for(int i=0; i<CLIENTES;i++)
			cond_clientes.signal();
   	for(int i=0; i<BARBEROS;i++)
			cond_barbero[i].signal();
	}
}

void Barberia::finCliente( int barbero )
{
  	cout << "Barbero despide al Cliente." << endl << flush;
   cond_silla[barbero].signal();
   cond_clientes.signal();
}

// -----------------------------------------------------------------------------

void  funcion_hebra_cliente( MRef<Barberia> monitor, int num_cliente )
{
   while( open )
   {
      monitor->cortarPelo( num_cliente );
      EsperarFueraBarberia( num_cliente );
   }
}

void funcion_hebra_barbero( MRef<Barberia> monitor, int num_barbero )
{
   while( open ){
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
   cout << endl << "En la peluquería hoy se han facturado " << aux << " cortes de pelo." << endl;
}
