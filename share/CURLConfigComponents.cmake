  foreach(z_vcpkg_curl_component IN ITEMS alt-svc AsynchDNS IPv6 Largefile libz threadsafe UnixSockets HTTP)
    if(z_vcpkg_curl_component MATCHES "^[-_a-zA-Z0-9]*$")
      set(CURL_${z_vcpkg_curl_component}_FOUND TRUE)
    endif()
  endforeach()
  