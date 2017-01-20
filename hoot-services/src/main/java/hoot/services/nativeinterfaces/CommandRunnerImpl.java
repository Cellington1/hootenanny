/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2016 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.nativeinterfaces;


import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.time.LocalDateTime;
import java.util.Arrays;

import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.ExecuteStreamHandler;
import org.apache.commons.exec.ExecuteWatchdog;
import org.apache.commons.exec.Executor;
import org.apache.commons.exec.PumpStreamHandler;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


/**
 * Utility class for running a subprocess synchronously from Java.
 */
public class CommandRunnerImpl implements CommandRunner {
    private static final Logger logger = LoggerFactory.getLogger(CommandRunnerImpl.class);

    private ExecuteWatchdog watchDog;
    private OutputStream stdout;
    private OutputStream stderr;

    public CommandRunnerImpl() {}

    @Override
    public CommandResult exec(String[] command) {
        logger.debug("Executing the following command: {}", Arrays.toString(command));

        try (OutputStream stdout = new ByteArrayOutputStream();
             OutputStream stderr = new ByteArrayOutputStream()) {

            LocalDateTime start = LocalDateTime.now();

            logger.debug("Command {} started at {}", Arrays.toString(command), start);

            this.stdout = stdout;
            this.stderr = stderr;

            CommandLine cmdLine = new CommandLine(command[0]);
            for (int i = 1; i < command.length; i++) {
                cmdLine.addArgument(command[i], false);
            }

            ExecuteStreamHandler executeStreamHandler = new PumpStreamHandler(stdout, stderr);
            Executor executor = new DefaultExecutor();
            this.watchDog = new ExecuteWatchdog(ExecuteWatchdog.INFINITE_TIMEOUT);
            executor.setWatchdog(this.watchDog);
            executor.setStreamHandler(executeStreamHandler);

            int exitCode;
            try {
                exitCode = executor.execute(cmdLine);

                if (executor.isFailure(exitCode) && this.watchDog.killedProcess()) {
                    // it was killed on purpose by the watchdog
                    logger.info("Process for '{}' command was killed!", cmdLine);
                }
            }
            catch (Exception e) {
                exitCode = CommandResult.FAILURE;
                logger.warn("Error executing: {}", cmdLine, e);
            }

            LocalDateTime finish = LocalDateTime.now();

            CommandResult commandResult = new CommandResult(cmdLine.toString(), exitCode,
                    stdout.toString(), stderr.toString());
            commandResult.setStart(start);
            commandResult.setFinish(finish);

            logger.debug("Command {} finished at {}", Arrays.toString(command), finish);

            return commandResult;
        }
        catch (Exception e) {
            throw new RuntimeException("Error executing " + Arrays.toString(command) + " command!", e);
        }
    }

    @Override
    public String getStdout() {
        return this.stdout.toString();
    }

    @Override
    public void terminate() {
        if (!this.watchDog.killedProcess()) {
            this.watchDog.destroyProcess();
        }
    }
}