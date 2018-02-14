/*
 * ActiveModule.h
 *
 *  Versión: 14 Feb 2018
 *  Author: raulMrello
 *
 *	Changelog: 
 *	- @14Feb2018.001 Cambio 'ready=true' una vez que se haya completado el evento Init::EV_ENTRY.
 *
 *	ActiveModule es un interfaz que proporciona características comunes de funcionamiento relativas a los módulos de
 *	gestión de alto nivel.
 *
 *	Incluyen su propio hilo de ejecución, su topic_base de publicación y de suscripción, métodos comunes de acceso a
 *	datos almacenados en memoria NV.
 */
 
#ifndef __ActiveModule__H
#define __ActiveModule__H

#include "mbed.h"
#include "StateMachine.h"
#include "MQLib.h"
#include "FSManager.h"

class ActiveModule : public StateMachine {
  public:
              
    /** Constructor, que asocia un nombre, así como el tamaño de stack necesario para el thread
     *  @param name Nombre del módulo
     *  @param priority Prioridad del thread asociado
     *  @param stack_size Tamaño de stack asociado al thread
     */
    ActiveModule(const char* name, osPriority priority=osPriorityNormal, uint32_t stack_size = OS_STACK_SIZE, FSManager* fs = NULL, bool defdbg = false);


    /** Destructor
     */
    virtual ~ActiveModule(){}


    /** Chequea si el módulo está preparado, es decir su thread está corriendo.
     * 	@return True: thread corriendo, False: Inicializando
     */
    bool ready() { return _ready; }


    /** Chequea si el módulo tiene las trazas de depuración activadas
     * 	@return True: activadas, False: desactivadas
     */
    bool degugActive() { return _defdbg; }
  
  
    /** Configura el topic base para la publicación de mensajes
     *  @param pub_topic_base Topic base para la publicación
     */
    void setPublicationBase(const char* pub_topic_base){
    	_pub_topic_base = pub_topic_base;
    }
    

    /** Configura el topic base para la suscripción de mensajes
     *  @param sub_topic_base Topic base para la suscripción
     */
    void setSubscriptionBase(const char* sub_topic_base){
    	_sub_topic_base = sub_topic_base;
    }


    /** Interfaz para postear un mensaje de la máquina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual void putMessage(State::Msg *msg) = 0;


  protected:

    const char* _pub_topic_base;				/// Nombre del topic base para las publicaciones
    const char* _sub_topic_base;				/// Nombre del topic base para las suscripciones
    bool _defdbg;								/// Flag para depuración por defecto
    MQ::SubscribeCallback   _subscriptionCb;    /// Callback de suscripción a topics
    MQ::PublishCallback     _publicationCb;     /// Callback de publicación en topics

  private:

    static const uint8_t MaxNameLength = 16;	/// Tamaño del nombre
    Thread* _th;								/// Thread asociado al módulo
    bool _ready;								/// Flag para indicar el estado del módulo a nivel de thread
    char _name[MaxNameLength+1];				/// Nombre del módulo (ej. "[Name]..........")

    FSManager* _fs;								/// Gestor del sistema de backup en memoria NVS

    State _stInit;								/// Variable de estado para stInit



  protected:

    /** Rutina de entrada a la máquina de estados (gestionada por la clase heredera)
     */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se) = 0;


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent() = 0;


	/** Callback invocada al recibir una actualización de un topic local al que está suscrito
     *  @param topic Identificador del topic
     *  @param msg Mensaje recibido
     *  @param msg_len Tamaño del mensaje
     */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len) = 0;


	/** Callback invocada al finalizar una publicación local
     *  @param topic Identificador del topic
     *  @param result Resultado de la publicación
     */
    virtual void publicationCb(const char* topic, int32_t result) = 0;


   	/** Chequea la integridad de los datos de configuración <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity() = 0;


   	/** Establece la configuración por defecto grabándola en memoria NV
	 */
	virtual void setDefaultConfig() = 0;


   	/** Recupera la configuración de memoria NV
	 */
	virtual void restoreConfig() = 0;


   	/** Graba la configuración en memoria NV
	 */
	virtual void saveConfig() = 0;

	/** Graba un parámetro en la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Datos asociados
	 * 	@param size Tamaño de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo salvar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);


	/** Recupera un parámetro de la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tamaño de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);


  private:

    /** Hilo de ejecución propio.
     */
    void task();

};
     
#endif /*__ActiveModule__H */

/**** END OF FILE ****/


