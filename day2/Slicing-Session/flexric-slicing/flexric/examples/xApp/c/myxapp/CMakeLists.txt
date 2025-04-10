add_executable(xapp_rc_slice_ho
	xapp_rc_slice_ho.c
	kpm.c
	rc_slice.c
	rc_ho_slice.c
	../../../../src/util/alg_ds/alg/defer.c
	)

add_executable(xapp_create_slice
xapp_create_slice.c
	kpm.c
	rc_slice.c
	rc_ho_slice.c
	../../../../src/util/alg_ds/alg/defer.c
	)
add_executable(xapp_slice_handover
xapp_slice_handover.c
	kpm.c
	rc_slice.c
	rc_ho_slice.c
	../../../../src/util/alg_ds/alg/defer.c
	)
target_link_libraries(xapp_rc_slice_ho
											PUBLIC
											e42_xapp
											-pthread
											-lsctp
											-ldl
											cjson # Use full path to avoid linker error
											)


		target_link_libraries(xapp_create_slice
											PUBLIC
											e42_xapp
											-pthread
											-lsctp
											-ldl
											cjson # Use full path to avoid linker error
											)
		target_link_libraries(xapp_slice_handover
											PUBLIC
											e42_xapp
											-pthread
											-lsctp
											-ldl
											cjson # Use full path to avoid linker error
											)