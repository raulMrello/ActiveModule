/*
 * TemplateImpl.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */

#include "TemplateImpl.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(ActiveModule::_defdbg){					\
	printf(format, ##__VA_ARGS__);			\
}											\
 
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
TemplateImpl::TemplateImpl(FSManager* fs, bool defdbg) : ActiveModule("TemplateImpl", osPriorityNormal, OS_STACK_SIZE, fs, defdbg) {
	_publicationCb = callback(this, &TemplateImpl::publicationCb);
	_subscriptionCb = callback(this, &TemplateImpl::subscriptionCb);
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void TemplateImpl::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // procesa un evento, por ejemplo "xxx/which/event"
    if(MQ::MQClient::isTopicToken(topic, "/which/event")){
        DEBUG_TRACE("\r\nTemplImp\t Recibido topic %s", topic);

        bool chk_ok = false;
		/* Chequea que el mensaje tiene formato correcto */
		//TODO
		
		if(!chk_ok){
			DEBUG_TRACE("\r\nTemplImp\t ERR_MSG, mensaje con formato incorrecto en topic '%s'", topic);
			return;
		}
				
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);
		
		/* Reserva espacio para alojar el mensaje y parsea el mensaje */
		//TODO Heap::memAlloc(...);

		/* Asigna el tipo de señal (evento) */
		//TODO
        op->sig = WhichEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		//TODO
        op->msg = NULL;
		
        // postea en la cola de la máquina de estados
        _queue.put(op);
        return;
    }
    DEBUG_TRACE("\r\nTemplImp\t ERR_TOPIC. No se puede procesar el topic '%s'", topic);
}

//------------------------------------------------------------------------------------
State::StateResult TemplateImpl::Init_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	int err = osOK;
        	DEBUG_TRACE("\r\nTemplImp\t Iniciando recuperación de datos...");
        	// recupera los datos de memoria NV
        	err = _fs->restore("TemplateImplCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
        	if(err == osOK){
            	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
            	// almacenándolos de nuevo en memoria NV.
            	if(!checkIntegrity()){
            		DEBUG_TRACE("\r\nTemplImp\t ERR_CFG. Ha fallado el check de integridad. Establece configuración por defecto.");
					setDefaultConfig();
            	}
				else{
					DEBUG_TRACE("\r\nTemplImp\t Recuperación de datos OK!");
				}
        	}
			else{
				DEBUG_TRACE("\r\nTemplImp\t ERR_FS. Error en la recuperación de datos. Establece configuración por defecto");
				setDefaultConfig();
			}
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            return State::HANDLED;
        }

        // Procesa datos recibidos relativos al evento WhichEvt
        case WhichEvt:{
        	/* Recupera el mensaje */
			//TODO var* data = (var*)(st_msg->msg);

        	/* Si es necesario, almacena en el sistema de ficheros la configuración o el parámetro correspondiente */
			//TODO: _fs->save("TemplImplCfg", &_cfg.updFlagMask, sizeof(Config), NVSInterface::TypeBlob);
			//TODO: _fs->saveParameter("TemplImplParam", &_cfg.param, sizeof(param), NVSInterface::TypeParam);
        	DEBUG_TRACE("\r\n[AstCal]\t TemplImp\t Datos actualizados");
			
			/* Si es necesario publica actualización en topic */
			//TODO
			char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "%s/which/topic", _pub_topic_base);
			Data* pmsg = (Data*)Heap::memAlloc(sizeof(Data));
			MBED_ASSERT(pmsg);
			memcpy(pmsg, &data_src, size);
			MQ::MQClient::publish(pub_topic, pmsg, sizeof(Data), &_publicationCb);
			Heap::memFree(pmsg);
			Heap::memFree(pub_topic);
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);
			
			/* Si es necesario cambia de estado */
			//TODO
			tranState(stSiguiente);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


//------------------------------------------------------------------------------------
void TemplateImpl::putMessage(State::Msg *msg){
    _queue.put(msg);
}


//------------------------------------------------------------------------------------
osEvent TemplateImpl:: getOsEvent(){
	return _queue.get();
}



//------------------------------------------------------------------------------------
void TemplateImpl::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool TemplateImpl::checkIntegrity(){
	bool chk_ok = true;
	/* Chequea integridad de la configuración */
	// TODO
	
	if(!chk_ok){
		setDefaultConfig();
		return false;
	}	
	return true;
}


//------------------------------------------------------------------------------------
void TemplateImpl::setDefaultConfig(){
	/* Establece configuración por defecto */
	//TODO
	
	/* Guarda en memoria NV */
	//TODO
	_fs->save("TemplImplCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
	_fs->saveParameter("TemplImplParam", &_cfg.param, sizeof(Param), NVSInterface::TypeParam);
}

