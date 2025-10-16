#ifndef __InactiveModule__H
#define __InactiveModule__H

#include "mbed.h"
#include "MQLib.h"
#include "StateMachine.h"
#include "FSManager.h"
#include "ActiveModule.h"
#include "GlobalActiveModule.h"

class ActiveModule;

class InactiveModule : public StateMachine, public GlobalActiveModule {
  public:
              
    /** Constructor, que asocia un nombre, as� como el tama�o de stack necesario para el thread
     *  @param name Nombre del m�dulo
     *  @param priority Prioridad del thread asociado
     *  @param stack_size Tama�o de stack asociado al thread
     */
    InactiveModule(const char* name, FSManager* fs = NULL, bool defdbg = false, bool logActive = false, const char* logName = "ActiveMod");


    /** Destructor
     */
    virtual ~InactiveModule(){}


    /** Chequea si el m�dulo est� preparado, es decir su thread est� corriendo.
     * 	@return True: thread corriendo, False: Inicializando
     */
    bool ready();


    /** Chequea si el m�dulo tiene las trazas de depuraci�n activadas
     * 	@return True: activadas, False: desactivadas
     */
    bool debugActive() { return _defdbg; }

    void start();
  
  
    /** Configura el topic base para la publicaci�n de mensajes
     *  @param pub_topic_base Topic base para la publicaci�n
     */
    void setPublicationBase(const char* pub_topic_base){
    	_pub_topic_base = pub_topic_base;
      
        if(_sub_topic_base != NULL){
            start();
        }
    }
    

    /** Configura el topic base para la suscripci�n de mensajes
     *  @param sub_topic_base Topic base para la suscripci�n
     */
    void setSubscriptionBase(const char* sub_topic_base){
    	_sub_topic_base = sub_topic_base;
      
        if(_pub_topic_base != NULL){
            start();
        }
    }


    /** Registra su operativa en el gestor watchdog de tareas
     * 	@param millis Temporizaci�n para enviar el ping de actividad
     * 	@param wdg_topic Topic en el que publicar la notificaci�n de actividad
     * 	@param name Nombre utilizado para publicar
     */
    void attachToTaskWatchdog(uint32_t millis, const char* wdg_topic, const char* name);


    /** Interfaz para postear un mensaje de la m�quina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     *  @return Resultado
     */
    virtual osStatus putMessage(State::Msg *msg);

    void setActiveModule(ActiveModule* parent);

    void checkActiveHandlers(State::StateEvent* se);

    bool checkActiveHandlers(osEvent oe);

    ActiveModule* getActiveModule(){return _activeModule;}

    void initStateMachine(){
      // asigna m�quina de estados por defecto  y la inicia
      initState(&_stInit);
    }


  protected:
    //int32_t _queue_count;
    //static int32_t _max_queue_count;

    /** Tiempo de espera por defecto al postear un mensaje */
    //static const uint32_t DefaultPutTimeout = MQ::MQBroker::DefaultMutexTimeout;

    const char* _pub_topic_base;				/// Nombre del topic base para las publicaciones
    const char* _sub_topic_base;				/// Nombre del topic base para las suscripciones
    bool _defdbg;								/// Flag para depuraci�n por defecto
    MQ::SubscribeCallback   _subscriptionCb;    /// Callback de suscripci�n a topics
    MQ::PublishCallback     _publicationCb;     /// Callback de publicaci�n en topics
    FSManager* _fs;								/// Gestor del sistema de backup en memoria NVS
    bool _ready;								/// Flag para indicar el estado del m�dulo a nivel de thread
    //bool _wdt_handled;							/// Flag para indicar si debe reportar al TaskWatchdog
    //uint32_t _wdt_millis;						/// Cadencia en ms para notificar actividad al TaskWatchdog
    //char* _wdt_topic;							/// Topic en el que publicar la notificaci�n de actividad
    //char* _wdt_name;							/// Nombre del componente que publica la notificaci�n

    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    //static const uint32_t DefaultMaxQueueMessages = 48;

    /** Cola de mensajes de la m�quina de estados */
    //Queue<State::Msg, DefaultMaxQueueMessages> _queue;


    State _stInit;								/// Variable de estado para stInit


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


    /** Rutina de entrada a la m�quina de estados (gestionada por la clase heredera)
     */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se) = 0;


	/** Callback invocada al recibir una actualizaci�n de un topic local al que est� suscrito
     *  @param topic Identificador del topic
     *  @param msg Mensaje recibido
     *  @param msg_len Tama�o del mensaje
     */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len) = 0;


	/** Callback invocada al finalizar una publicaci�n local
     *  @param topic Identificador del topic
     *  @param result Resultado de la publicaci�n
     */
    virtual void publicationCb(const char* topic, int32_t result) = 0;


   	/** Chequea la integridad de los datos de configuraci�n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity() = 0;


   	/** Establece la configuraci�n por defecto grab�ndola en memoria NV
	 */
	virtual void setDefaultConfig() = 0;


   	/** Recupera la configuraci�n de memoria NV
	 */
	virtual void restoreConfig() = 0;


   	/** Graba la configuraci�n en memoria NV
	 */
	virtual void saveConfig() = 0;

	/** Graba un par�metro en la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Datos asociados
	 * 	@param size Tama�o de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo salvar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);


	/** Recupera un par�metro de la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tama�o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type);

  /* Elimina un parametro de la memoria NV
   * @param param_id Identificador del parametro
   * @return True: exito, False: no se pudo recuperar
  */
  virtual bool removeParameter(const char* param_id);

  void checkInactiveModules(State::StateEvent* se);


  private:

    static const uint8_t MaxNameLength = 16;	/// Tama�o del nombre
    //Thread* _th;								/// Thread asociado al m�dulo
    char _name[MaxNameLength+1];				/// Nombre del m�dulo (ej. "[Name]..........")
    //Semaphore _sem_th{0,1};
    ActiveModule* _activeModule;						/// M�dulo activo asociado
    list<State::EventHandler> handlersList;            /// Lista de manejadores de eventos

    /** Hilo de ejecuci�n propio.
     */
    void task();

};
     
#endif /*__InactiveModule__H */

/**** END OF FILE ****/


