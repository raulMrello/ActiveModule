/*
 * TaskTaskWatchdog.h
 *
 *  Versi�n: Dic 2019
 *  Author: raulMrello
 *
 *	Componente que utiliza publicaciones en MQLib para recibir los keep-alive de los componentes que se hayan
 *	registrado. Cuando un componente no actualiza su keep-alive en el plazo establecido se notificar� el error
 *	como una publicaci�n en el topic stat/timeout/$TWD indicando el nombre del componente que ha fallado, adem�s
 *	se invocar� a la callback instalada en caso de timeout en el plazo establecido.
 */

#ifndef __TASKWATCHDOG_H
#define __TASKWATCHDOG_H

#include "mbed.h"
#include "MQLib.h"
#include <map>


class TaskWatchdog   {
public:

	/**
	 * Constructor
	 * @param name Nombre del componente utilizado para sus publicaciones
	 * @param keepalive_text Texto publicado por los componentes para notificar un keepalive
	 */
	TaskWatchdog(const char* name, const char* keepalive_text, esp_log_level_t level = ESP_LOG_INFO);

    /**
     * Destructor
     */
    ~TaskWatchdog();


    /**
     * Instala una callback a invocar cuando se produce un error por timeout. El proceso al detectar
     * un timeout de alg�n componente registrado, como sigue:
     * 1) Notificar error en el topic "stat/timeout/$TWD" -m(nombre componentes csv)
     * 2) Esperar (pre_wait) millis
     * 3) Ejecutar cb
     * @param pre_wait Milisegundos a esperar tras la publicaci�n y antes de la callback
     * @param cb Callbacka ejecutar en caso de fallo por timeout
     */
    void attachTimeoutCb(uint32_t pre_wait, Callback<void()> cb){
    	_err_pre_callback = pre_wait;
    	_err_cb = cb;
    }

    /**
     * Inicia la ejecuci�n del TaskWatchdog, liberando la lista de componentes registrados
     */
    void start(uint32_t timeout);


    /**
     * Detiene la ejecuci�n
     */
    void stop();

    /**
     * Obtiene la temporizaci�n del TWD
     * @return Timeout en ms
     */
    uint32_t getTimeout() {
    	return _timeout;
    }

    /**
     * Chequea si est� en ejecuci�n
     * @return
     */
    bool isRunning() {
    	return _running;
    }

private:

    /** Nombre de publicaci�n del TWD*/
    const char* _name;

    /** Texto de publicaci�n del keepalive */
    const char* _ka_text;

    /** Mapa de componentes registrados */
    std::map<std::string, uint32_t> _components;

    /** Flag para indicar si est� operativo */
    bool _running;

    /** Timer de ejecuci�n del TaskWatchdog */
    RtosTimer* _tmr;

    /** Temporizaci�n del timer */
    uint32_t _timeout;

    /** Tiempo a esperar antes de ejecutar la callback tras fallo */
    uint32_t _err_pre_callback;
    Callback<void()> _err_cb;

    /** Publicador y suscription */
    MQ::PublishCallback _publicationCb;
    MQ::SubscribeCallback _subscriptionCb;

    /**
     * Elimina el mapa de componentes registrados
     */
    void _eraseRegisteredComponents();

    /**
     * Crea el timer
     */
    void _createTimer();

    /**
     * Callback del timer asociado al TWD
     */
    void _onTick();

    /**
     * Suscriptor de las notificaciones de keep alive
     * @param name Topic
     * @param msg Mensaje (nombre del componente)
     * @param msg_len Tama�o del mensaje
     */
    void _onSubscription(const char* name, void* msg, uint16_t msg_len);

    /**
     * Publicador
     * @param name Topic
     * @param err C�digo de error
     */
    void _onPublished(const char* name, int32_t err){
    }
};


#endif // __TASKWATCHDOG_H
