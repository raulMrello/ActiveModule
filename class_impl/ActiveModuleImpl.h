/*
 * TemplateImpl.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	TemplateImpl es una plantilla para implementar una clase que hereda de ActiveModule
 */
 
#ifndef __TemplateImpl__H
#define __TemplateImpl__H

#include "mbed.h"
#include "ActiveModule.h"


   
class TemplateImpl : public ActiveModule {
  public:
              
    /** Constructor por defecto
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuración por defecto
     */
    TemplateImpl(FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~TemplateImpl(){}

  protected:

    /** Máximo número de mensajes alojables en la cola asociada a la máquina de estados */
    static const uint32_t MaxQueueMessages = 16;


    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	WhichEvt = (State::EV_RESERVED_USER << 0),  /// Flag inicial
		/* Añadir otros aquí */
    };

    /** Cola de mensajes de la máquina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuración */
	struct Config {
		uint8_t color[3];	// Color RGB
		uint8_t speed;		// Velocidad de 0..100
	};
	Config _cfg;


    /** Interfaz para postear un mensaje de la máquina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual void putMessage(State::Msg *msg);


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


 	/** Interfaz para manejar los eventos en la máquina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualización de un topic local al que está suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tamaño del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicación local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicación
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuración <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	bool checkIntegrity();


   	/** Establece la configuración por defecto grabándola en memoria NV
	 */
	void setDefaultConfig();

};
     
#endif /*__TemplateImpl__H */

/**** END OF FILE ****/


