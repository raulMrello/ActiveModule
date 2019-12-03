/*
 * ActiveModule.h
 *
 *  Versiï¿½n: 7 Mar 2018
 *  Author: raulMrello
 *
 *	Changelog: 
 *	- @7Mar2018.001 Habilito DefaultPutTimeout para evitar dead-locks ocultos en mutex.lock
 *	- @14Feb2018.001 Cambio 'ready=true' una vez que se haya completado el evento Init::EV_ENTRY.
 *
 *	ActiveModule es un interfaz que proporciona caracterï¿½sticas comunes de funcionamiento relativas a los mï¿½dulos de
 *	gestiï¿½n de alto nivel.
 *
 *	Incluyen su propio hilo de ejecuciï¿½n, su topic_base de publicaciï¿½n y de suscripciï¿½n, mï¿½todos comunes de acceso a
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
              
    /** Constructor, que asocia un nombre, asï¿½ como el tamaï¿½o de stack necesario para el thread
     *  @param name Nombre del mï¿½dulo
     *  @param priority Prioridad del thread asociado
     *  @param stack_size Tamaï¿½o de stack asociado al thread
     */
    ActiveModule(const char* name, osPriority priority=osPriorityNormal, uint32_t stack_size = OS_STACK_SIZE, FSManager* fs = NULL, bool defdbg = false);


    /** Destructor
     */
    virtual ~ActiveModule(){}


    /** Chequea si el mï¿½dulo estï¿½ preparado, es decir su thread estï¿½ corriendo.
     * 	@return True: thread corriendo, False: Inicializando
     */
    bool ready() { return _ready; }


    /** Chequea si el mï¿½dulo tiene las trazas de depuraciï¿½n activadas
     * 	@return True: activadas, False: desactivadas
     */
    bool degugActive() { return _defdbg; }
  
  
    /** Configura el topic base para la publicaciï¿½n de mensajes
     *  @param pub_topic_base Topic base para la publicaciï¿½n
     */
    void setPublicationBase(const char* pub_topic_base){
    	_pub_topic_base = pub_topic_base;
    }
    

    /** Configura el topic base para la suscripciï¿½n de mensajes
     *  @param sub_topic_base Topic base para la suscripciï¿½n
     */
    void setSubscriptionBase(const char* sub_topic_base){
    	_sub_topic_base = sub_topic_base;
    }


	#if ESP_PLATFORM == 1
    /** Registra su operativa en el gestor watchdog de tareas
     * 	@param millis Temporización para enviar el ping de actividad
     */
    void attachToTaskWatchdog(uint32_t millis);
	#endif


	#if ESP_PLATFORM == 1
	/** Desregistra su operativa del gestor watchdog de tareas
	 */
	void detachFromTaskWatchdog();
	#endif

    /** Interfaz para postear un mensaje de la mï¿½quina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     *  @return Resultado
     */
    virtual osStatus putMessage(State::Msg *msg);


  protected:

    /** Tiempo de espera por defecto al postear un mensaje */
    static const uint32_t DefaultPutTimeout = MQ::MQBroker::DefaultMutexTimeout;

    const char* _pub_topic_base;				/// Nombre del topic base para las publicaciones
    const char* _sub_topic_base;				/// Nombre del topic base para las suscripciones
    bool _defdbg;								/// Flag para depuraciï¿½n por defecto
    MQ::SubscribeCallback   _subscriptionCb;    /// Callback de suscripciï¿½n a topics
    MQ::PublishCallback     _publicationCb;     /// Callback de publicaciï¿½n en topics
    FSManager* _fs;								/// Gestor del sistema de backup en memoria NVS
    bool _ready;								/// Flag para indicar el estado del mï¿½dulo a nivel de thread
    bool _wdt_handled;							/// Flag para indicar si debe reportar al TaskWatchdog
    uint32_t _wdt_millis;						/// Cadencia en ms para notificar actividad al TaskWatchdog

    /** Máximo número de mensajes alojables en la cola asociada a la máquina de estados */
    static const uint32_t DefaultMaxQueueMessages = 16;

    /** Cola de mensajes de la máquina de estados */
    Queue<State::Msg, DefaultMaxQueueMessages> _queue;


  private:

    static const uint8_t MaxNameLength = 16;	/// Tamaï¿½o del nombre
    Thread* _th;								/// Thread asociado al mï¿½dulo
    char _name[MaxNameLength+1];				/// Nombre del mï¿½dulo (ej. "[Name]..........")
    Semaphore _sem_th{0,1};



  protected:

    State _stInit;								/// Variable de estado para stInit


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


    /** Rutina de entrada a la mï¿½quina de estados (gestionada por la clase heredera)
     */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se) = 0;


	/** Callback invocada al recibir una actualizaciï¿½n de un topic local al que estï¿½ suscrito
     *  @param topic Identificador del topic
     *  @param msg Mensaje recibido
     *  @param msg_len Tamaï¿½o del mensaje
     */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len) = 0;


	/** Callback invocada al finalizar una publicaciï¿½n local
     *  @param topic Identificador del topic
     *  @param result Resultado de la publicaciï¿½n
     */
    virtual void publicationCb(const char* topic, int32_t result) = 0;


   	/** Chequea la integridad de los datos de configuraciï¿½n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity() = 0;


   	/** Establece la configuraciï¿½n por defecto grabï¿½ndola en memoria NV
	 */
	virtual void setDefaultConfig() = 0;


   	/** Recupera la configuraciï¿½n de memoria NV
	 */
	virtual void restoreConfig() = 0;


   	/** Graba la configuraciï¿½n en memoria NV
	 */
	virtual void saveConfig() = 0;

	/** Graba un parï¿½metro en la memoria NV
	 * 	@param param_id Identificador del parï¿½metro
	 * 	@param data Datos asociados
	 * 	@param size Tamaï¿½o de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: ï¿½xito, False: no se pudo salvar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);


	/** Recupera un parï¿½metro de la memoria NV
	 * 	@param param_id Identificador del parï¿½metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tamaï¿½o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: ï¿½xito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);


	#if ESP_PLATFORM == 1
	/** Notifica al TaskWatchdog que está operativa
	 */
	void _wdogKick(){
		esp_task_wdt_reset();
	}
	#endif


  private:

    /** Hilo de ejecuciï¿½n propio.
     */
    void task();

};
     
#endif /*__ActiveModule__H */

/**** END OF FILE ****/


