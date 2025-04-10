add_executable(xapp_kpm # xapp_rc_slice_ho
	# xapp_rc_slice_ho.c
	kpm.c
	rc_slice.c
	rc_ho_slice.c
	xapp_kpm.c
	../../../../src/util/alg_ds/alg/defer.c
	)

target_link_libraries(xapp_kpm # xapp_rc_slice_ho
											PUBLIC
											e42_xapp
											-pthread
											-lsctp
											-ldl
											)

