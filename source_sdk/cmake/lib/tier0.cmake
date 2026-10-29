block()
	set( lib_name tier0 )
	set( lib_dir ${SOURCE_SDK_ROOT}/lib/public )

	add_library( ${lib_name} SHARED IMPORTED GLOBAL )

	if( WIN32 )
			set_property( TARGET ${lib_name} PROPERTY IMPORTED_IMPLIB ${lib_dir}/${lib_name}.lib )
		elseif( APPLE )
			set_property( TARGET ${lib_name} PROPERTY IMPORTED_LOCATION ${lib_dir}/lib${lib_name}.dylib )
		else()
			set_property( TARGET ${lib_name} PROPERTY IMPORTED_LOCATION ${lib_dir}/lib${lib_name}.so )
	endif()
endblock()
