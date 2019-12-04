/*
 * TaskTaskWatchdog.h
 *
 *  Versión: Dic 2019
 *  Author: raulMrello
 *
 *	Componente que utiliza publicaciones en MQLib para recibir los keep-alive de los componentes que se hayan
 *	registrado. Cuando un componente no actualiza su keep-alive en el plazo establecido se notificará el error
 *	como una publicación en el topic stat/timeout/$TWD indicando el nombre del componente que ha fallado, además
 *	se invocará a la callback instalada en caso de timeout en el plazo establecido.
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
     * un timeout de algún componente registrado, como sigue:
     * 1) Notificar error en el topic "stat/timeout/$TWD" -m(nombre componentes csv)
     * 2) Esperar (pre_wait) millis
     * 3) Ejecutar cb
     * @param pre_wait Milisegundos a esperar tras la publicación y antes de la callback
     * @param cb Callbacka ejecutar en caso de fallo por timeout
     */
    void attachTimeoutCb(uint32_t pre_wait, Callback<void()> cb){
    	_err_pre_callback = pre_wait;
    	_err_cb = cb;
    }

    /**
     * Inicia la ejecución del TaskWatchdog, liberando la lista de componentes registrados
     */
    void start(uint32_t timeout);


    /**
     * Detiene la ejecución
     */
    void stop();

    /**
     * Obtiene la temporización del TWD
     * @return Timeout en ms
     */
    uint32_t getTimeout() {
    	return _timeout;
    }

    /**
     * Chequea si está en ejecución
     * @return
     */
    bool isRunning() {
    	return _running;
    }

private:

    /** Nombre de publicación del TWD*/
    const char* _name;

    /** Texto de publicación del keepalive */
    const char* _ka_text;

    /** Mapa de componentes registrados */
    std::map<std::string, uint32_t> _components;

    /** Flag para indicar si está operativo */
    bool _running;

    /** Timer de ejecución del TaskWatchdog */
    RtosTimer* _tmr;

    /** Temporización del timer */
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
     * @param msg_len Tamaño del mensaje
     */
    void _onSubscription(const char* name, void* msg, uint16_t msg_len);

    /**
     * Publicador
     * @param name Topic
     * @param err Código de error
     */
    void _onPublished(const char* name, int32_t err){
    }
};


#endif // __TASKWATCHDOG_H
