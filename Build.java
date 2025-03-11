import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Duration;
import java.time.LocalDateTime;
import java.time.Period;

public class Build {

    private static final String DIR = System.getProperty("user.dir");

    public static void main(String[] args) {
        LocalDateTime now = LocalDateTime.now();
        Process p1 = exec(Path.of(DIR), new String[]{"git", "--version"});
        try {
            int r = p1.waitFor();
            if(r != 0) {
                throw new RuntimeException("Git not found");
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while detecting git", e);
        }

        Process p2 = exec(Path.of(DIR), new String[]{"clang", "--version"});
        try {
            int r = p2.waitFor();
            if(r != 0) {
                throw new RuntimeException("Clang not found");
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while detecting clang", e);
        }

        Path thirdPartyPath = createDir("thirdparty");

        if(Files.exists(thirdPartyPath.resolve("wepoll"))) {
            System.out.println("wepoll already exists");
        } else {
            Process p3 = exec(thirdPartyPath, new String[]{"git", "clone", "--depth=1", "--single-branch", "--branch", "jing", "https://github.com/benrush0705/wepoll.git"});
            try {
                int r = p3.waitFor();
                if(r == 0) {
                    System.out.println("wepoll successfully cloned");
                }
            } catch (InterruptedException e) {
                throw new RuntimeException("Interrupted while cloning wepoll", e);
            }
        }

        if(Files.exists(thirdPartyPath.resolve("rpmalloc"))) {
            System.out.println("rpmalloc already exists");
        } else {
            Process p4 = exec(thirdPartyPath, new String[]{"git", "clone", "--depth=1", "--single-branch", "--branch", "jing", "https://github.com/benrush0705/rpmalloc.git"});
            try {
                int r = p4.waitFor();
                if(r == 0) {
                    System.out.println("rpmalloc successfully cloned");
                }
            } catch (InterruptedException e) {
                throw new RuntimeException("Interrupted while cloning rpmalloc", e);
            }
        }

        Path srcPath = Path.of(DIR, "src");
        Path _ = createDir("lib");
        Process p5 = exec(srcPath, createCompilerCommand());
        try{
            int r = p5.waitFor();
            if(r != 0) {
                throw new RuntimeException("Compilation failed with exit code : " + r);
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while compilation", e);
        }

        System.out.println("\nCompilation successfully completed, total build cost : %d milli seconds".formatted(Duration.between(now, LocalDateTime.now()).toMillis()));
    }

    private static Path createDir(String name) {
        Path p = Path.of(DIR, name);
        if (Files.exists(p)) {
            if(!Files.isDirectory(p)) {
                throw new RuntimeException("%s dir exist, but not a directory".formatted(name));
            }
            return p;
        }
        try{
            System.out.println("Creating %s directory...".formatted(name));
            return Files.createDirectory(p);
        } catch (IOException e) {
            throw new RuntimeException("%s dir could not be created".formatted(name), e);
        }
    }

    private static Process exec(Path path, String[] cmd) {
        String command = String.join(" ", cmd);
        try{
            System.out.println("Executing : " + command);
            ProcessBuilder processBuilder = new ProcessBuilder(cmd);
            processBuilder.inheritIO();
            processBuilder.directory(path.toFile());
            Process process = processBuilder.start();
            return process;
        } catch (IOException e) {
            throw new RuntimeException("Failed to exec command : " + command, e);
        }
    }

    private static String[] createCompilerCommand() {
        String osName = System.getProperty("os.name").toLowerCase();
        if(osName.contains("windows")) {
            return new String[]{"clang", "-std=c17", "-Wall", "-shared", "-march=native", "-O3", "-g0", "-fcolor-diagnostics", "-fansi-escape-codes",
                    "-v", ".\\mem.c", "..\\thirdparty\\rpmalloc\\rpmalloc\\rpmalloc.c", "..\\thirdparty.\\wepoll.\\wepoll.c", "-lAdvapi32", "-lws2_32", "-pedantic", "-o", "..\\lib\\jing.dll"};
        }else if(osName.contains("linux")) {
            return new String[]{};
        } else if (osName.contains("mac") && osName.contains("os")) {
            return new String[]{};
        }else {
            throw new RuntimeException("Unsupported OS: " + osName);
        }
    }
}