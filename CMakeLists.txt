cmake_minimum_required(VERSION 3.28)
project(EndingsDetector LANGUAGES CXX)

include(CheckIPOSupported)

add_executable(EndingsDetector main.cpp)
target_compile_features(EndingsDetector PRIVATE cxx_std_23)

check_ipo_supported(RESULT result OUTPUT error)
if(result)
    set_target_properties(EndingsDetector PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()

if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    set_property(TARGET EndingsDetector PROPERTY
                MSVC_RUNTIME_LIBRARY "$<IF:$<CONFIG:Debug>,MultiThreadedDebug,MultiThreaded>")

    target_compile_options(EndingsDetector PRIVATE
        $<IF:$<CONFIG:Debug>,/Od,/O2>
        /GR-                            # Disable RTTI
        $<IF:$<CONFIG:Debug>,/GS,/GS->  # Disable security checks
        /GL                             # Whole program optimization
        /EHs-                           # No exceptions
        /fp:fast

        #/Gs9999999
    )

    target_compile_definitions(EndingsDetector PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )

    target_link_options(EndingsDetector PRIVATE
        #$<$<NOT:$<CONFIG:Debug>>:/ALIGN:16>
        /EMITPOGOPHASEINFO:NO
        /EMITTOOLVERSIONINFO:NO

        ##/SUBSYSTEM:CONSOLE,5.01
        ##/GUARD:NO
        ##/MERGE:.rdata=.text
        ##/ORDER:@order.txt
        ## /NOCOFFGRPINFO    # Remove COFF group info
        ## /NONAMEPROMOTE    # Remove name promotion
        ## /FILEALIGN:16     # File alignment in PE
        ## /SAFESEH:NO       # Disable safe exception handlers
        ## /TLBID:1          # Minimal type library ID
        ## /SWAPRUN:NET      # Disable swapping для .NET
        ## /SWAPRUN:CD       # Disable swapping для CD
        ## /TSAWARE:NO       # Disable Terminal Server awareness
        ## /VERSION:0.0      # Minimal version
        ## /LARGEADDRESSAWARE:NO  # Disable large address awareness

        /STACK:8388608 # 8 Mb
        /OPT:REF
        /OPT:ICF
        /NODEFAULTLIB
        /IGNORE:4099
        /INCREMENTAL:NO
        /MANIFEST:NO
        /LTCG
        #/ENTRY:WinMainCRTStartup
        #/ENTRY:main
    )

    target_link_libraries(EndingsDetector PRIVATE
        $<IF:$<CONFIG:Debug>,
            libcmtd.lib
            libvcruntimed.lib
            libucrtd.lib
            ,
            libcmt.lib
            #libvcruntimed.lib
            #libucrtd.lib
        >
    )
endif()
