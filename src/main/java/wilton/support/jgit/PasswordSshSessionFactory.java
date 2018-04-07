/*
 * Copyright 2016, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package wilton.support.jgit;

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
