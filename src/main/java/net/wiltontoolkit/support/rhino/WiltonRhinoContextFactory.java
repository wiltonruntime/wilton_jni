package net.wiltontoolkit.support.rhino;

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
