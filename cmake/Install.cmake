install(DIRECTORY include/zg DESTINATION ${ZG_INC_INSTALL_PREFIX}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

install(TARGETS zeungine
    ARCHIVE DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    LIBRARY DESTINATION ${ZG_LIB_INSTALL_PREFIX})
install(TARGETS zedit
    DESTINATION ${ZG_BIN_INSTALL_PREFIX})
if((LINUX OR MACOS) AND "${CMAKE_BUILD_TYPE}" MATCHES "Release")
    get_filename_component(ZG_BIN_INSTALL_PREFIX_UP2 "${ZG_BIN_INSTALL_PREFIX}/../.." ABSOLUTE)
    install(CODE "execute_process(COMMAND rm ${ZG_BIN_INSTALL_PREFIX_UP2}/zedit)")
    install(CODE "execute_process(COMMAND ln -s ${ZG_BIN_INSTALL_PREFIX}/zedit ${ZG_BIN_INSTALL_PREFIX_UP2}/zedit)")
endif()
if(WINDOWS)
    set(PATH_TO_ADD "C:\\\\Program Files\\\\Zeungine\\\\${CMAKE_BUILD_TYPE}")
    install(CODE "
        execute_process(
            COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"
                \$dirToAdd='${PATH_TO_ADD}';
                \$envPath=[System.Environment]::GetEnvironmentVariable('Path', [System.EnvironmentVariableTarget]::User);
                if (-not \$envPath -or \$envPath.Split(';') -notcontains \$dirToAdd) {
                    \$newPath = (\$envPath + ';' + \$dirToAdd).Trim(';');
                    [System.Environment]::SetEnvironmentVariable('Path', \$newPath, [System.EnvironmentVariableTarget]::User);
                }
            \"
        )
    ")
endif()