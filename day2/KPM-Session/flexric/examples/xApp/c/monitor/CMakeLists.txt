include_directories(/path/to/yaml/include)
link_directories(/path/to/yaml/lib)

add_executable(xapp_kpm_moni 
    xapp_kpm_moni.c
        ../../../../src/util/alg_ds/alg/defer.c
        )

target_link_libraries(xapp_kpm_moni
            PUBLIC
            e42_xapp
            -pthread
            -lsctp
            -ldl
            yaml
            )

add_executable(xapp_gtp_mac_rlc_pdcp_moni 
    xapp_gtp_mac_rlc_pdcp_moni.c
        ../../../../src/util/alg_ds/alg/defer.c
        )

target_link_libraries(xapp_gtp_mac_rlc_pdcp_moni
            PUBLIC
            e42_xapp
            -pthread
            -lsctp
            -ldl
            yaml
            )

add_executable(xapp_rc_moni 
         xapp_rc_moni.c
         ../../../../src/util/alg_ds/alg/defer.c
        )
          
target_link_libraries(xapp_rc_moni
           PUBLIC
           e42_xapp
           -pthread
           -lsctp
           -ldl
           yaml
           )
