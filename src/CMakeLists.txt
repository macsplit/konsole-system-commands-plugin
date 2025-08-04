kcoreaddons_add_plugin(konsole_systemcommandsplugin
SOURCES
    systemcommandsplugin.cpp
INSTALL_NAMESPACE
    "konsoleplugins"
)

target_link_libraries(konsole_systemcommandsplugin
    konsoleprivate
    konsoleapp
)