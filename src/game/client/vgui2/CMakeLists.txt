# VGUI2 layer. Source SDK headers clash with the client's own (Vector, interface.h, FileSystem.h...),
# so they are only visible to this library. The client talks to it through ms_vgui2.h.
add_library(client_vgui2 STATIC
	CClientVGUI.cpp
	CClientVGUI.h
	CClientViewport.cpp
	CClientViewport.h
	IViewportPanel.h
	ms_vgui2.cpp
	ms_vgui2.h
	chat/CChatPanel.cpp
	chat/CChatPanel.h)

set_common_properties(client_vgui2)

# Same standard as the SDK libraries, its headers predate C++20
set_target_properties(client_vgui2 PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

target_include_directories(client_vgui2 PRIVATE
	${SOURCE_SDK_INCLUDE_PATHS}
	${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(client_vgui2 PRIVATE
	${SOURCE_SDK_DEFINES})

target_link_libraries(client_vgui2 PRIVATE
	${SOURCE_SDK_LIBS})
