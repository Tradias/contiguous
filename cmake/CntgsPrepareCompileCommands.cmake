# args: WORKSPACE_ROOT

set(NEW_WORKSPACE_ROOT_REGEX "\\/github\\/workspace\\/")
string(REPLACE "/" "\\/" REGEX_WORKSPACE_ROOT "${WORKSPACE_ROOT}")
execute_process(COMMAND sed -i "s/${REGEX_WORKSPACE_ROOT}/${NEW_WORKSPACE_ROOT_REGEX}/g"
                        "${WORKSPACE_ROOT}/build/compile_commands.json")
