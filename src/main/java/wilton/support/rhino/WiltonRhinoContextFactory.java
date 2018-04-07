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

package wilton.support.rhino;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ContextFactory;

/**
 * See https://github.com/mozilla/rhino/issues/153
 *
 * User: alexkasko
 * Date: 10/17/16
 */
class WiltonRhinoContextFactory extends ContextFactory {

    static WiltonRhinoContextFactory INSTANCE = new WiltonRhinoContextFactory();

    private WiltonRhinoContextFactory() {
    }

    @Override
    protected boolean hasFeature(Context cx, int featureIndex) {
        if (Context.FEATURE_LOCATION_INFORMATION_IN_ERROR == featureIndex) {
            return true;
        }
        return super.hasFeature(cx, featureIndex);
    }

    @Override
    protected void onContextCreated(Context cx) {
        cx.setOptimizationLevel(-1);
        cx.setGeneratingDebug(true);
        super.onContextCreated(cx);
    }
}
