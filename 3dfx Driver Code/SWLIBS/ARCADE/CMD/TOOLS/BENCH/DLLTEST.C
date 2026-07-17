#if 0
    /*-----------------------------------------------------------------
      Test DLL Loading
      -----------------------------------------------------------------*/
    {
        HINSTANCE hDll;
        hDll = LoadLibrary( "lightdlg\\test.dll" );
        if ( !hDll ) {
            LPVOID lpMsgBuf;
            
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL 
                          );
            
            // Display the string.
            MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
            
            // Free the buffer.
            LocalFree( lpMsgBuf );
            return -1;
        } else {
            typedef void (func)(void);
            func *fptr;
            FARPROC proc;
            
            proc = GetProcAddress( hDll, "testInit2" );
            if ( !proc ) {
                LPVOID lpMsgBuf;
                
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              GetLastError(),
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                              (LPTSTR) &lpMsgBuf,
                              0,
                              NULL );
                
                // Display the string.
                MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
                
                // Free the buffer.
                LocalFree( lpMsgBuf );
                return -1;
            } else {
                fptr = (func*)proc;
                fptr();
            }
            FreeLibrary( hDll );
        }
    }
#endif

