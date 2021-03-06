cmake_minimum_required(VERSION 3.0.0)
project(CppExplorer)

set(DIALOGS_SRC
	dialogs/appchooserdialog.cxx
	dialogs/helpdialog.cxx
	dialogs/helpdialog.ui
	dialogs/keyboardshortcutsdialog.cxx
	dialogs/keyboardshortcutsdialog.ui
)

set(MENU_SRC
	menu/background_contextmenu.cxx
	menu/file_contextmenu.cxx
	menu/folder_contextmenu.cxx
	menu/multi_contextmenu.cxx
)

set(MENUBAR_SRC
	menubar/editmenu.cxx
	menubar/filemenu.cxx
	menubar/helpmenu.cxx
	menubar/viewmenu.cxx
)

set(BASE_SRC
	icons.qrc
	rsc.qrc
	actions.cxx
	addressbar_buttons.cxx
	addressbar_text.cxx
	browserwidget.cxx
	endtoolbar.cxx
	main.cxx
	navbar.cxx
	searchbar.cxx
	sidebar.cxx
	tabwidget.cxx
	tinyxml2.cpp
	trash.cxx
	trashbar.cxx
	window.cxx
)

set(SRC
	${DIALOGS_SRC}
	${MENU_SRC}
	${MENUBAR_SRC}
	${BASE_SRC}
)

add_executable(CppExplorer ${SRC})

target_link_libraries(CppExplorer
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	settings
)

if (UNIX)
	install(TARGETS CppExplorer DESTINATION /usr/local/bin)
endif()
