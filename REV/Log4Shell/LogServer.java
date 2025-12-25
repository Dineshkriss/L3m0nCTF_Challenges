import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import java.util.Scanner;

public class LogServer {
    private static final Logger logger = LogManager.getLogger(LogServer.class);

    public static void main(String[] args) {
        System.out.println("LogServer v1.0 (Patched)");
        System.out.println("Enter log message:");
        
        Scanner scanner = new Scanner(System.in);
        if (scanner.hasNextLine()) {
            String input = scanner.nextLine();
            handleLog(input);
        }
    }

    public static void handleLog(String input) {
        // The "Native Filter" Patch
        if (NativeSecurity.check(input)) {
            logger.warn("Malicious input blocked: " + input);
            System.out.println("Access Denied: Malicious Pattern Detected.");
        } else {
            // If safe, we log it (simulating the vulnerability trigger)
            logger.info("Received: " + input);
            
            // In a real challenge, this is where the RCE would happen.
            // For this RE challenge, reaching this line with a payload
            // that *should* be blocked means the user bypassed the filter.
            if (input.contains("${") && input.contains("}")) {
                System.out.println("Congratulations! You bypassed the filter.");
                System.out.println("Flag: CTF{" + input + "}");
            }
        }
    }
}
