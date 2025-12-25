package org.apache.logging.log4j;

public class Logger {
    private String name;

    public Logger(String name) {
        this.name = name;
    }

    public void info(String msg) {
        System.out.println("[INFO] " + name + ": " + msg);
    }

    public void warn(String msg) {
        System.out.println("[WARN] " + name + ": " + msg);
    }

    public void error(String msg) {
        System.out.println("[ERROR] " + name + ": " + msg);
    }
}
