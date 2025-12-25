public class NativeSecurity {
    // This 'native' keyword tells Java to look for this
    // function in a loaded C/C++ library
    public static native boolean check(String input);

    static {
        // This loads 'libsecurity.so' into memory
        System.loadLibrary("security"); 
    }
}
