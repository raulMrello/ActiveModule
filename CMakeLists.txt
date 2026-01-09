idf_component_register(	SRCS "ActiveModule.cpp" "InactiveModule.cpp"
						INCLUDE_DIRS "."
						REQUIRES mbed_api_esp32 StateMachine FSManager)