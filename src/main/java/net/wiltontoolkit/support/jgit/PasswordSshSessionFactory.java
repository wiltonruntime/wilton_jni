package net.wiltontoolkit.support.jgit;

import com.jcraft.jsch.Session;
import com.jcraft.jsch.UserInfo;
import org.eclipse.jgit.errors.UnsupportedCredentialItem;
import org.eclipse.jgit.transport.*;

/**
 * User: alexkasko
 * Date: 5/19/16
 */
public class PasswordSshSessionFactory extends JschConfigSessionFactory {
    private final String password;
    private boolean strictHostKeyChecking = true;

    public PasswordSshSessionFactory(String password) {
        this.password = password;
    }

    public PasswordSshSessionFactory withStrictHostKeyChecking(boolean value) {
        this.strictHostKeyChecking = value;
        return this;
    }

    @Override
    protected void configure(OpenSshConfig.Host hc, Session session) {
        String val = strictHostKeyChecking ? "yes" : "no";
        session.setConfig("StrictHostKeyChecking", val);
        Provider prov = new Provider(password);
        UserInfo userInfo = new CredentialsProviderUserInfo(session, prov);
        session.setUserInfo(userInfo);
    }

    private static class Provider extends CredentialsProvider {

        private String password;

        public Provider(String password) {
            this.password = password;
        }

        @Override
        public boolean isInteractive() {
            return false;
        }

        @Override
        public boolean supports(CredentialItem... items) {
            return true;
        }

        @Override
        public boolean get(URIish uri, CredentialItem... items) throws UnsupportedCredentialItem {
            for (CredentialItem it : items) {
                if (it instanceof CredentialItem.Password) {
                    CredentialItem.Password pwd = (CredentialItem.Password) it;
                    pwd.setValue(password.toCharArray());
                }
            }
            return true;
        }
    }
}
