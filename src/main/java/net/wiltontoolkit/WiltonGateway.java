package net.wiltontoolkit;

/**
 * User: alexkasko
 * Date: 1/12/17
 */
public interface WiltonGateway {

    /**
     * Implementation should run the script specified in the provided description:
     * {@code
     *     {
     *         "module": "some/module/name",
     *         "func": "someFunction",
     *         "args": [...]
     *     }
     * }
     *
     *
     * @param data script description
     * @throws Exception on call fail
     * @return script output
     */
    String runScript(String data) throws Exception;
}
