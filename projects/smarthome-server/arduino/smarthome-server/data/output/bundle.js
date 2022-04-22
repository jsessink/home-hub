
(function(l, r) { if (!l || l.getElementById('livereloadscript')) return; r = l.createElement('script'); r.async = 1; r.src = '//' + (self.location.host || 'localhost').split(':')[0] + ':35729/livereload.js?snipver=1'; r.id = 'livereloadscript'; l.getElementsByTagName('head')[0].appendChild(r) })(self.document);
var app = (function () {
    'use strict';

    function noop() { }
    function is_promise(value) {
        return value && typeof value === 'object' && typeof value.then === 'function';
    }
    function add_location(element, file, line, column, char) {
        element.__svelte_meta = {
            loc: { file, line, column, char }
        };
    }
    function run(fn) {
        return fn();
    }
    function blank_object() {
        return Object.create(null);
    }
    function run_all(fns) {
        fns.forEach(run);
    }
    function is_function(thing) {
        return typeof thing === 'function';
    }
    function safe_not_equal(a, b) {
        return a != a ? b == b : a !== b || ((a && typeof a === 'object') || typeof a === 'function');
    }
    function is_empty(obj) {
        return Object.keys(obj).length === 0;
    }
    function append(target, node) {
        target.appendChild(node);
    }
    function insert(target, node, anchor) {
        target.insertBefore(node, anchor || null);
    }
    function detach(node) {
        node.parentNode.removeChild(node);
    }
    function destroy_each(iterations, detaching) {
        for (let i = 0; i < iterations.length; i += 1) {
            if (iterations[i])
                iterations[i].d(detaching);
        }
    }
    function element(name) {
        return document.createElement(name);
    }
    function text(data) {
        return document.createTextNode(data);
    }
    function space() {
        return text(' ');
    }
    function empty() {
        return text('');
    }
    function listen(node, event, handler, options) {
        node.addEventListener(event, handler, options);
        return () => node.removeEventListener(event, handler, options);
    }
    function attr(node, attribute, value) {
        if (value == null)
            node.removeAttribute(attribute);
        else if (node.getAttribute(attribute) !== value)
            node.setAttribute(attribute, value);
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function custom_event(type, detail, bubbles = false) {
        const e = document.createEvent('CustomEvent');
        e.initCustomEvent(type, bubbles, false, detail);
        return e;
    }

    let current_component;
    function set_current_component(component) {
        current_component = component;
    }
    function get_current_component() {
        if (!current_component)
            throw new Error('Function called outside component initialization');
        return current_component;
    }
    function onMount(fn) {
        get_current_component().$$.on_mount.push(fn);
    }

    const dirty_components = [];
    const binding_callbacks = [];
    const render_callbacks = [];
    const flush_callbacks = [];
    const resolved_promise = Promise.resolve();
    let update_scheduled = false;
    function schedule_update() {
        if (!update_scheduled) {
            update_scheduled = true;
            resolved_promise.then(flush);
        }
    }
    function add_render_callback(fn) {
        render_callbacks.push(fn);
    }
    // flush() calls callbacks in this order:
    // 1. All beforeUpdate callbacks, in order: parents before children
    // 2. All bind:this callbacks, in reverse order: children before parents.
    // 3. All afterUpdate callbacks, in order: parents before children. EXCEPT
    //    for afterUpdates called during the initial onMount, which are called in
    //    reverse order: children before parents.
    // Since callbacks might update component values, which could trigger another
    // call to flush(), the following steps guard against this:
    // 1. During beforeUpdate, any updated components will be added to the
    //    dirty_components array and will cause a reentrant call to flush(). Because
    //    the flush index is kept outside the function, the reentrant call will pick
    //    up where the earlier call left off and go through all dirty components. The
    //    current_component value is saved and restored so that the reentrant call will
    //    not interfere with the "parent" flush() call.
    // 2. bind:this callbacks cannot trigger new flush() calls.
    // 3. During afterUpdate, any updated components will NOT have their afterUpdate
    //    callback called a second time; the seen_callbacks set, outside the flush()
    //    function, guarantees this behavior.
    const seen_callbacks = new Set();
    let flushidx = 0; // Do *not* move this inside the flush() function
    function flush() {
        const saved_component = current_component;
        do {
            // first, call beforeUpdate functions
            // and update components
            while (flushidx < dirty_components.length) {
                const component = dirty_components[flushidx];
                flushidx++;
                set_current_component(component);
                update(component.$$);
            }
            set_current_component(null);
            dirty_components.length = 0;
            flushidx = 0;
            while (binding_callbacks.length)
                binding_callbacks.pop()();
            // then, once components are updated, call
            // afterUpdate functions. This may cause
            // subsequent updates...
            for (let i = 0; i < render_callbacks.length; i += 1) {
                const callback = render_callbacks[i];
                if (!seen_callbacks.has(callback)) {
                    // ...so guard against infinite loops
                    seen_callbacks.add(callback);
                    callback();
                }
            }
            render_callbacks.length = 0;
        } while (dirty_components.length);
        while (flush_callbacks.length) {
            flush_callbacks.pop()();
        }
        update_scheduled = false;
        seen_callbacks.clear();
        set_current_component(saved_component);
    }
    function update($$) {
        if ($$.fragment !== null) {
            $$.update();
            run_all($$.before_update);
            const dirty = $$.dirty;
            $$.dirty = [-1];
            $$.fragment && $$.fragment.p($$.ctx, dirty);
            $$.after_update.forEach(add_render_callback);
        }
    }
    const outroing = new Set();
    let outros;
    function group_outros() {
        outros = {
            r: 0,
            c: [],
            p: outros // parent group
        };
    }
    function check_outros() {
        if (!outros.r) {
            run_all(outros.c);
        }
        outros = outros.p;
    }
    function transition_in(block, local) {
        if (block && block.i) {
            outroing.delete(block);
            block.i(local);
        }
    }
    function transition_out(block, local, detach, callback) {
        if (block && block.o) {
            if (outroing.has(block))
                return;
            outroing.add(block);
            outros.c.push(() => {
                outroing.delete(block);
                if (callback) {
                    if (detach)
                        block.d(1);
                    callback();
                }
            });
            block.o(local);
        }
    }

    function handle_promise(promise, info) {
        const token = info.token = {};
        function update(type, index, key, value) {
            if (info.token !== token)
                return;
            info.resolved = value;
            let child_ctx = info.ctx;
            if (key !== undefined) {
                child_ctx = child_ctx.slice();
                child_ctx[key] = value;
            }
            const block = type && (info.current = type)(child_ctx);
            let needs_flush = false;
            if (info.block) {
                if (info.blocks) {
                    info.blocks.forEach((block, i) => {
                        if (i !== index && block) {
                            group_outros();
                            transition_out(block, 1, 1, () => {
                                if (info.blocks[i] === block) {
                                    info.blocks[i] = null;
                                }
                            });
                            check_outros();
                        }
                    });
                }
                else {
                    info.block.d(1);
                }
                block.c();
                transition_in(block, 1);
                block.m(info.mount(), info.anchor);
                needs_flush = true;
            }
            info.block = block;
            if (info.blocks)
                info.blocks[index] = block;
            if (needs_flush) {
                flush();
            }
        }
        if (is_promise(promise)) {
            const current_component = get_current_component();
            promise.then(value => {
                set_current_component(current_component);
                update(info.then, 1, info.value, value);
                set_current_component(null);
            }, error => {
                set_current_component(current_component);
                update(info.catch, 2, info.error, error);
                set_current_component(null);
                if (!info.hasCatch) {
                    throw error;
                }
            });
            // if we previously had a then/catch block, destroy it
            if (info.current !== info.pending) {
                update(info.pending, 0);
                return true;
            }
        }
        else {
            if (info.current !== info.then) {
                update(info.then, 1, info.value, promise);
                return true;
            }
            info.resolved = promise;
        }
    }
    function update_await_block_branch(info, ctx, dirty) {
        const child_ctx = ctx.slice();
        const { resolved } = info;
        if (info.current === info.then) {
            child_ctx[info.value] = resolved;
        }
        if (info.current === info.catch) {
            child_ctx[info.error] = resolved;
        }
        info.block.p(child_ctx, dirty);
    }

    const globals = (typeof window !== 'undefined'
        ? window
        : typeof globalThis !== 'undefined'
            ? globalThis
            : global);
    function create_component(block) {
        block && block.c();
    }
    function mount_component(component, target, anchor, customElement) {
        const { fragment, on_mount, on_destroy, after_update } = component.$$;
        fragment && fragment.m(target, anchor);
        if (!customElement) {
            // onMount happens before the initial afterUpdate
            add_render_callback(() => {
                const new_on_destroy = on_mount.map(run).filter(is_function);
                if (on_destroy) {
                    on_destroy.push(...new_on_destroy);
                }
                else {
                    // Edge case - component was destroyed immediately,
                    // most likely as a result of a binding initialising
                    run_all(new_on_destroy);
                }
                component.$$.on_mount = [];
            });
        }
        after_update.forEach(add_render_callback);
    }
    function destroy_component(component, detaching) {
        const $$ = component.$$;
        if ($$.fragment !== null) {
            run_all($$.on_destroy);
            $$.fragment && $$.fragment.d(detaching);
            // TODO null out other refs, including component.$$ (but need to
            // preserve final state?)
            $$.on_destroy = $$.fragment = null;
            $$.ctx = [];
        }
    }
    function make_dirty(component, i) {
        if (component.$$.dirty[0] === -1) {
            dirty_components.push(component);
            schedule_update();
            component.$$.dirty.fill(0);
        }
        component.$$.dirty[(i / 31) | 0] |= (1 << (i % 31));
    }
    function init(component, options, instance, create_fragment, not_equal, props, append_styles, dirty = [-1]) {
        const parent_component = current_component;
        set_current_component(component);
        const $$ = component.$$ = {
            fragment: null,
            ctx: null,
            // state
            props,
            update: noop,
            not_equal,
            bound: blank_object(),
            // lifecycle
            on_mount: [],
            on_destroy: [],
            on_disconnect: [],
            before_update: [],
            after_update: [],
            context: new Map(options.context || (parent_component ? parent_component.$$.context : [])),
            // everything else
            callbacks: blank_object(),
            dirty,
            skip_bound: false,
            root: options.target || parent_component.$$.root
        };
        append_styles && append_styles($$.root);
        let ready = false;
        $$.ctx = instance
            ? instance(component, options.props || {}, (i, ret, ...rest) => {
                const value = rest.length ? rest[0] : ret;
                if ($$.ctx && not_equal($$.ctx[i], $$.ctx[i] = value)) {
                    if (!$$.skip_bound && $$.bound[i])
                        $$.bound[i](value);
                    if (ready)
                        make_dirty(component, i);
                }
                return ret;
            })
            : [];
        $$.update();
        ready = true;
        run_all($$.before_update);
        // `false` as a special case of no DOM component
        $$.fragment = create_fragment ? create_fragment($$.ctx) : false;
        if (options.target) {
            if (options.hydrate) {
                const nodes = children(options.target);
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.l(nodes);
                nodes.forEach(detach);
            }
            else {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.c();
            }
            if (options.intro)
                transition_in(component.$$.fragment);
            mount_component(component, options.target, options.anchor, options.customElement);
            flush();
        }
        set_current_component(parent_component);
    }
    /**
     * Base class for Svelte components. Used when dev=false.
     */
    class SvelteComponent {
        $destroy() {
            destroy_component(this, 1);
            this.$destroy = noop;
        }
        $on(type, callback) {
            const callbacks = (this.$$.callbacks[type] || (this.$$.callbacks[type] = []));
            callbacks.push(callback);
            return () => {
                const index = callbacks.indexOf(callback);
                if (index !== -1)
                    callbacks.splice(index, 1);
            };
        }
        $set($$props) {
            if (this.$$set && !is_empty($$props)) {
                this.$$.skip_bound = true;
                this.$$set($$props);
                this.$$.skip_bound = false;
            }
        }
    }

    function dispatch_dev(type, detail) {
        document.dispatchEvent(custom_event(type, Object.assign({ version: '3.46.5' }, detail), true));
    }
    function append_dev(target, node) {
        dispatch_dev('SvelteDOMInsert', { target, node });
        append(target, node);
    }
    function insert_dev(target, node, anchor) {
        dispatch_dev('SvelteDOMInsert', { target, node, anchor });
        insert(target, node, anchor);
    }
    function detach_dev(node) {
        dispatch_dev('SvelteDOMRemove', { node });
        detach(node);
    }
    function listen_dev(node, event, handler, options, has_prevent_default, has_stop_propagation) {
        const modifiers = options === true ? ['capture'] : options ? Array.from(Object.keys(options)) : [];
        if (has_prevent_default)
            modifiers.push('preventDefault');
        if (has_stop_propagation)
            modifiers.push('stopPropagation');
        dispatch_dev('SvelteDOMAddEventListener', { node, event, handler, modifiers });
        const dispose = listen(node, event, handler, options);
        return () => {
            dispatch_dev('SvelteDOMRemoveEventListener', { node, event, handler, modifiers });
            dispose();
        };
    }
    function attr_dev(node, attribute, value) {
        attr(node, attribute, value);
        if (value == null)
            dispatch_dev('SvelteDOMRemoveAttribute', { node, attribute });
        else
            dispatch_dev('SvelteDOMSetAttribute', { node, attribute, value });
    }
    function set_data_dev(text, data) {
        data = '' + data;
        if (text.wholeText === data)
            return;
        dispatch_dev('SvelteDOMSetData', { node: text, data });
        text.data = data;
    }
    function validate_each_argument(arg) {
        if (typeof arg !== 'string' && !(arg && typeof arg === 'object' && 'length' in arg)) {
            let msg = '{#each} only iterates over array-like objects.';
            if (typeof Symbol === 'function' && arg && Symbol.iterator in arg) {
                msg += ' You can use a spread to convert this iterable into an array.';
            }
            throw new Error(msg);
        }
    }
    function validate_slots(name, slot, keys) {
        for (const slot_key of Object.keys(slot)) {
            if (!~keys.indexOf(slot_key)) {
                console.warn(`<${name}> received an unexpected slot "${slot_key}".`);
            }
        }
    }
    /**
     * Base class for Svelte components with some minor dev-enhancements. Used when dev=true.
     */
    class SvelteComponentDev extends SvelteComponent {
        constructor(options) {
            if (!options || (!options.target && !options.$$inline)) {
                throw new Error("'target' is a required option");
            }
            super();
        }
        $destroy() {
            super.$destroy();
            this.$destroy = () => {
                console.warn('Component was already destroyed'); // eslint-disable-line no-console
            };
        }
        $capture_state() { }
        $inject_state() { }
    }

    var ControllerType;
    (function (ControllerType) {
        ControllerType[ControllerType["PIR_RGB"] = 0] = "PIR_RGB";
    })(ControllerType || (ControllerType = {}));

    const isValidHex = (hex) => /^#([A-Fa-f0-9]{3,4}){1,2}$/.test(hex);
    const getChunksFromString = (st, chunkSize) => st.match(new RegExp(`.{${chunkSize}}`, "g"));
    const convertHexUnitTo256 = (hexStr) => parseInt(hexStr.repeat(2 / hexStr.length), 16);
    const getAlphaFloat = (a, alpha) => {
        if (typeof a !== "undefined") {
            return a / 255;
        }
        if ((typeof alpha != "number") || alpha < 0 || alpha > 1) {
            return 1;
        }
        return alpha;
    };
    const hexToRGBA = (hex, alpha) => {
        if (!isValidHex(hex)) {
            throw new Error("Invalid HEX");
        }
        const chunkSize = Math.floor((hex.length - 1) / 3);
        const hexArr = getChunksFromString(hex.slice(1), chunkSize);
        if (!hexArr) {
            throw new Error('Cannot obtain hex information');
        }
        const [r, g, b, a] = hexArr.map(convertHexUnitTo256);
        const alphaFloat = alpha ? getAlphaFloat(a, alpha) : 1;
        return { r, g, b, a: alphaFloat };
    };
    const rgbToHex = (r, g, b) => {
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
    };

    /* src\components\ControllerSettings\RGB-PIR.svelte generated by Svelte v3.46.5 */

    const { Object: Object_1 } = globals;
    const file$2 = "src\\components\\ControllerSettings\\RGB-PIR.svelte";

    // (80:2) {:catch error}
    function create_catch_block$1(ctx) {
    	let t0;
    	let t1;

    	const block = {
    		c: function create() {
    			t0 = text(/*controllerIP*/ ctx[0]);
    			t1 = text(" Offline");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, t0, anchor);
    			insert_dev(target, t1, anchor);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*controllerIP*/ 1) set_data_dev(t0, /*controllerIP*/ ctx[0]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(t0);
    			if (detaching) detach_dev(t1);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_catch_block$1.name,
    		type: "catch",
    		source: "(80:2) {:catch error}",
    		ctx
    	});

    	return block;
    }

    // (51:2) {:then rgbSettings}
    function create_then_block$1(ctx) {
    	let div;
    	let h2;
    	let t0_value = /*rgbSettings*/ ctx[6].deviceName + "";
    	let t0;
    	let t1;
    	let label0;
    	let span0;
    	let t3;
    	let input0;
    	let t4;
    	let label1;
    	let span1;
    	let t6;
    	let input1;
    	let t7;
    	let label2;
    	let span2;
    	let t9;
    	let input2;
    	let t10;
    	let label3;
    	let span3;
    	let t12;
    	let input3;
    	let t13;
    	let label4;
    	let span4;
    	let t15;
    	let input4;
    	let t16;
    	let label5;
    	let span5;
    	let t18;
    	let input5;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			div = element("div");
    			h2 = element("h2");
    			t0 = text(t0_value);
    			t1 = space();
    			label0 = element("label");
    			span0 = element("span");
    			span0.textContent = "Color";
    			t3 = space();
    			input0 = element("input");
    			t4 = space();
    			label1 = element("label");
    			span1 = element("span");
    			span1.textContent = "Always On?";
    			t6 = space();
    			input1 = element("input");
    			t7 = space();
    			label2 = element("label");
    			span2 = element("span");
    			span2.textContent = "Accidental Trip Delay (ms):";
    			t9 = space();
    			input2 = element("input");
    			t10 = space();
    			label3 = element("label");
    			span3 = element("span");
    			span3.textContent = "Duration On (ms):";
    			t12 = space();
    			input3 = element("input");
    			t13 = space();
    			label4 = element("label");
    			span4 = element("span");
    			span4.textContent = "Fade In Speed (ms):";
    			t15 = space();
    			input4 = element("input");
    			t16 = space();
    			label5 = element("label");
    			span5 = element("span");
    			span5.textContent = "Fade Out Speed (ms):";
    			t18 = space();
    			input5 = element("input");
    			attr_dev(h2, "class", "svelte-1sucoek");
    			add_location(h2, file$2, 52, 6, 1843);
    			attr_dev(span0, "class", "svelte-1sucoek");
    			add_location(span0, file$2, 55, 8, 1903);
    			attr_dev(input0, "data-setting", "color");
    			attr_dev(input0, "type", "color");
    			input0.value = /*rgbSettings*/ ctx[6].hex;
    			attr_dev(input0, "class", "svelte-1sucoek");
    			add_location(input0, file$2, 56, 8, 1931);
    			attr_dev(label0, "class", "svelte-1sucoek");
    			add_location(label0, file$2, 54, 6, 1886);
    			attr_dev(span1, "class", "svelte-1sucoek");
    			add_location(span1, file$2, 59, 8, 2067);
    			attr_dev(input1, "data-setting", "always-on");
    			attr_dev(input1, "type", "checkbox");
    			input1.checked = /*rgbSettings*/ ctx[6].alwaysOn;
    			attr_dev(input1, "class", "svelte-1sucoek");
    			add_location(input1, file$2, 60, 8, 2100);
    			attr_dev(label1, "class", "svelte-1sucoek");
    			add_location(label1, file$2, 58, 6, 2050);
    			attr_dev(span2, "class", "svelte-1sucoek");
    			add_location(span2, file$2, 63, 8, 2250);
    			attr_dev(input2, "data-setting", "acc-delay");
    			attr_dev(input2, "type", "number");
    			input2.value = /*rgbSettings*/ ctx[6].accidentalTripDelay;
    			attr_dev(input2, "class", "svelte-1sucoek");
    			add_location(input2, file$2, 64, 8, 2300);
    			attr_dev(label2, "class", "svelte-1sucoek");
    			add_location(label2, file$2, 62, 6, 2233);
    			attr_dev(span3, "class", "svelte-1sucoek");
    			add_location(span3, file$2, 67, 8, 2457);
    			attr_dev(input3, "data-setting", "duration-on");
    			attr_dev(input3, "type", "number");
    			input3.value = /*rgbSettings*/ ctx[6].durationOn;
    			attr_dev(input3, "class", "svelte-1sucoek");
    			add_location(input3, file$2, 68, 8, 2497);
    			attr_dev(label3, "class", "svelte-1sucoek");
    			add_location(label3, file$2, 66, 6, 2440);
    			attr_dev(span4, "class", "svelte-1sucoek");
    			add_location(span4, file$2, 71, 8, 2647);
    			attr_dev(input4, "data-setting", "in-speed");
    			attr_dev(input4, "type", "number");
    			input4.value = /*rgbSettings*/ ctx[6].fadeInSpeed;
    			attr_dev(input4, "class", "svelte-1sucoek");
    			add_location(input4, file$2, 72, 8, 2689);
    			attr_dev(label4, "class", "svelte-1sucoek");
    			add_location(label4, file$2, 70, 6, 2630);
    			attr_dev(span5, "class", "svelte-1sucoek");
    			add_location(span5, file$2, 75, 8, 2837);
    			attr_dev(input5, "data-setting", "out-speed");
    			attr_dev(input5, "type", "number");
    			input5.value = /*rgbSettings*/ ctx[6].fadeOutSpeed;
    			attr_dev(input5, "class", "svelte-1sucoek");
    			add_location(input5, file$2, 76, 8, 2880);
    			attr_dev(label5, "class", "svelte-1sucoek");
    			add_location(label5, file$2, 74, 6, 2820);
    			attr_dev(div, "class", "card svelte-1sucoek");
    			add_location(div, file$2, 51, 4, 1817);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, h2);
    			append_dev(h2, t0);
    			append_dev(div, t1);
    			append_dev(div, label0);
    			append_dev(label0, span0);
    			append_dev(label0, t3);
    			append_dev(label0, input0);
    			append_dev(div, t4);
    			append_dev(div, label1);
    			append_dev(label1, span1);
    			append_dev(label1, t6);
    			append_dev(label1, input1);
    			append_dev(div, t7);
    			append_dev(div, label2);
    			append_dev(label2, span2);
    			append_dev(label2, t9);
    			append_dev(label2, input2);
    			append_dev(div, t10);
    			append_dev(div, label3);
    			append_dev(label3, span3);
    			append_dev(label3, t12);
    			append_dev(label3, input3);
    			append_dev(div, t13);
    			append_dev(div, label4);
    			append_dev(label4, span4);
    			append_dev(label4, t15);
    			append_dev(label4, input4);
    			append_dev(div, t16);
    			append_dev(div, label5);
    			append_dev(label5, span5);
    			append_dev(label5, t18);
    			append_dev(label5, input5);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "change", /*onSettingChange*/ ctx[2], false, false, false),
    					listen_dev(input1, "change", /*onSettingChange*/ ctx[2], false, false, false),
    					listen_dev(input2, "change", /*onSettingChange*/ ctx[2], false, false, false),
    					listen_dev(input3, "change", /*onSettingChange*/ ctx[2], false, false, false),
    					listen_dev(input4, "change", /*onSettingChange*/ ctx[2], false, false, false),
    					listen_dev(input5, "change", /*onSettingChange*/ ctx[2], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_then_block$1.name,
    		type: "then",
    		source: "(51:2) {:then rgbSettings}",
    		ctx
    	});

    	return block;
    }

    // (49:24)       <p>... Getting online status and settings ...</p>    {:then rgbSettings}
    function create_pending_block$1(ctx) {
    	let p;

    	const block = {
    		c: function create() {
    			p = element("p");
    			p.textContent = "... Getting online status and settings ...";
    			add_location(p, file$2, 49, 4, 1739);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, p, anchor);
    		},
    		p: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(p);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_pending_block$1.name,
    		type: "pending",
    		source: "(49:24)       <p>... Getting online status and settings ...</p>    {:then rgbSettings}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$2(ctx) {
    	let div;

    	let info = {
    		ctx,
    		current: null,
    		token: null,
    		hasCatch: true,
    		pending: create_pending_block$1,
    		then: create_then_block$1,
    		catch: create_catch_block$1,
    		value: 6,
    		error: 7
    	};

    	handle_promise(/*settingsFetch*/ ctx[1], info);

    	const block = {
    		c: function create() {
    			div = element("div");
    			info.block.c();
    			attr_dev(div, "class", "rgb-picker svelte-1sucoek");
    			add_location(div, file$2, 47, 0, 1683);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			info.block.m(div, info.anchor = null);
    			info.mount = () => div;
    			info.anchor = null;
    		},
    		p: function update(new_ctx, [dirty]) {
    			ctx = new_ctx;
    			update_await_block_branch(info, ctx, dirty);
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			info.block.d();
    			info.token = null;
    			info = null;
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$2.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$2($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('RGB_PIR', slots, []);
    	let { controllerIP } = $$props;
    	let settingsFetch = getSettings();

    	async function getSettings() {
    		const res = await fetch(`/api/proxy/get-current-rgb-settings?ip=${controllerIP}`);
    		const body = await res.json();

    		// Check if ok and even just a single prop exists to show data was actually sent
    		if (res.ok && body.hasOwnProperty('r')) {
    			return Object.assign({ hex: rgbToHex(body.r, body.g, body.b) }, body);
    		}
    	}

    	onMount(() => {
    		
    	});

    	const onSettingChange = e => {
    		switch (e.target.dataset.setting) {
    			case 'color':
    				return colorChosen(e);
    			default:
    				const element = e.target;
    				let value;
    				switch (element.type) {
    					case 'checkbox':
    						value = element.checked;
    						break;
    					default:
    						value = element.value;
    						break;
    				}
    				return settingChange(e.target.dataset.setting, value);
    		}
    	};

    	async function colorChosen(e) {
    		var _a;

    		const rgb = hexToRGBA((_a = e.target) === null || _a === void 0
    		? void 0
    		: _a.value);

    		const res = await fetch(`/api/proxy/rgb?ip=${controllerIP}&r=${rgb.r}&g=${rgb.g}&b=${rgb.b}`, { method: "GET" });
    		await res.json();
    	}

    	async function settingChange(settingName, value) {
    		const res = await fetch(`/api/proxy/rgb-setting-change?ip=${controllerIP}&setting-name=${settingName}&setting-value=${value}`, { method: "GET" });
    		await res.json();
    	}

    	const writable_props = ['controllerIP'];

    	Object_1.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<RGB_PIR> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('controllerIP' in $$props) $$invalidate(0, controllerIP = $$props.controllerIP);
    	};

    	$$self.$capture_state = () => ({
    		onMount,
    		hexToRGBA,
    		rgbToHex,
    		controllerIP,
    		settingsFetch,
    		getSettings,
    		onSettingChange,
    		colorChosen,
    		settingChange
    	});

    	$$self.$inject_state = $$props => {
    		if ('controllerIP' in $$props) $$invalidate(0, controllerIP = $$props.controllerIP);
    		if ('settingsFetch' in $$props) $$invalidate(1, settingsFetch = $$props.settingsFetch);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [controllerIP, settingsFetch, onSettingChange];
    }

    class RGB_PIR extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$2, create_fragment$2, safe_not_equal, { controllerIP: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "RGB_PIR",
    			options,
    			id: create_fragment$2.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*controllerIP*/ ctx[0] === undefined && !('controllerIP' in props)) {
    			console.warn("<RGB_PIR> was created without expected prop 'controllerIP'");
    		}
    	}

    	get controllerIP() {
    		throw new Error("<RGB_PIR>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set controllerIP(value) {
    		throw new Error("<RGB_PIR>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src\components\ControllerList.svelte generated by Svelte v3.46.5 */
    const file$1 = "src\\components\\ControllerList.svelte";

    function get_each_context(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[2] = list[i];
    	return child_ctx;
    }

    // (1:0) <script lang="ts">import { onMount }
    function create_catch_block(ctx) {
    	const block = {
    		c: noop,
    		m: noop,
    		p: noop,
    		i: noop,
    		o: noop,
    		d: noop
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_catch_block.name,
    		type: "catch",
    		source: "(1:0) <script lang=\\\"ts\\\">import { onMount }",
    		ctx
    	});

    	return block;
    }

    // (20:40)       {#each controllers as controller}
    function create_then_block(ctx) {
    	let each_1_anchor;
    	let current;
    	let each_value = /*controllers*/ ctx[1];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block(get_each_context(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			each_1_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(target, anchor);
    			}

    			insert_dev(target, each_1_anchor, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*controllers$, ControllerType*/ 1) {
    				each_value = /*controllers*/ ctx[1];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(each_1_anchor.parentNode, each_1_anchor);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_each(each_blocks, detaching);
    			if (detaching) detach_dev(each_1_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_then_block.name,
    		type: "then",
    		source: "(20:40)       {#each controllers as controller}",
    		ctx
    	});

    	return block;
    }

    // (22:6) {#if controller.controllerType === ControllerType.PIR_RGB.toString()}
    function create_if_block(ctx) {
    	let rgbpir;
    	let current;

    	rgbpir = new RGB_PIR({
    			props: {
    				controllerIP: /*controller*/ ctx[2].controllerIP
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(rgbpir.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(rgbpir, target, anchor);
    			current = true;
    		},
    		p: noop,
    		i: function intro(local) {
    			if (current) return;
    			transition_in(rgbpir.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(rgbpir.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(rgbpir, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block.name,
    		type: "if",
    		source: "(22:6) {#if controller.controllerType === ControllerType.PIR_RGB.toString()}",
    		ctx
    	});

    	return block;
    }

    // (21:4) {#each controllers as controller}
    function create_each_block(ctx) {
    	let show_if = /*controller*/ ctx[2].controllerType === ControllerType.PIR_RGB.toString();
    	let if_block_anchor;
    	let current;
    	let if_block = show_if && create_if_block(ctx);

    	const block = {
    		c: function create() {
    			if (if_block) if_block.c();
    			if_block_anchor = empty();
    		},
    		m: function mount(target, anchor) {
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, if_block_anchor, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (show_if) if_block.p(ctx, dirty);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(if_block_anchor);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block.name,
    		type: "each",
    		source: "(21:4) {#each controllers as controller}",
    		ctx
    	});

    	return block;
    }

    // (1:0) <script lang="ts">import { onMount }
    function create_pending_block(ctx) {
    	const block = {
    		c: noop,
    		m: noop,
    		p: noop,
    		i: noop,
    		o: noop,
    		d: noop
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_pending_block.name,
    		type: "pending",
    		source: "(1:0) <script lang=\\\"ts\\\">import { onMount }",
    		ctx
    	});

    	return block;
    }

    function create_fragment$1(ctx) {
    	let div;
    	let current;

    	let info = {
    		ctx,
    		current: null,
    		token: null,
    		hasCatch: false,
    		pending: create_pending_block,
    		then: create_then_block,
    		catch: create_catch_block,
    		value: 1,
    		blocks: [,,,]
    	};

    	handle_promise(/*controllers$*/ ctx[0], info);

    	const block = {
    		c: function create() {
    			div = element("div");
    			info.block.c();
    			attr_dev(div, "class", "controller-list-container");
    			add_location(div, file$1, 18, 0, 532);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			info.block.m(div, info.anchor = null);
    			info.mount = () => div;
    			info.anchor = null;
    			current = true;
    		},
    		p: function update(new_ctx, [dirty]) {
    			ctx = new_ctx;
    			update_await_block_branch(info, ctx, dirty);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(info.block);
    			current = true;
    		},
    		o: function outro(local) {
    			for (let i = 0; i < 3; i += 1) {
    				const block = info.blocks[i];
    				transition_out(block);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			info.block.d();
    			info.token = null;
    			info = null;
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$1.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    async function getControllersList() {
    	const res = await fetch(`/api/get-controller-list`);
    	const body = await res.json();

    	if (res.ok) {
    		return body;
    	} else {
    		throw 'Something went wrong';
    	}
    }

    function instance$1($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('ControllerList', slots, []);
    	const controllers$ = getControllersList();

    	onMount(() => {
    		
    	});

    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<ControllerList> was created with unknown prop '${key}'`);
    	});

    	$$self.$capture_state = () => ({
    		onMount,
    		ControllerType,
    		RgbPir: RGB_PIR,
    		controllers$,
    		getControllersList
    	});

    	return [controllers$];
    }

    class ControllerList extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$1, create_fragment$1, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "ControllerList",
    			options,
    			id: create_fragment$1.name
    		});
    	}
    }

    /* src\App.svelte generated by Svelte v3.46.5 */
    const file = "src\\App.svelte";

    function create_fragment(ctx) {
    	let main;
    	let h1;
    	let t1;
    	let controllerlist;
    	let current;
    	controllerlist = new ControllerList({ $$inline: true });

    	const block = {
    		c: function create() {
    			main = element("main");
    			h1 = element("h1");
    			h1.textContent = "Sessink Home Hub";
    			t1 = space();
    			create_component(controllerlist.$$.fragment);
    			attr_dev(h1, "class", "svelte-7fgssi");
    			add_location(h1, file, 4, 1, 106);
    			attr_dev(main, "class", "svelte-7fgssi");
    			add_location(main, file, 3, 0, 97);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, main, anchor);
    			append_dev(main, h1);
    			append_dev(main, t1);
    			mount_component(controllerlist, main, null);
    			current = true;
    		},
    		p: noop,
    		i: function intro(local) {
    			if (current) return;
    			transition_in(controllerlist.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(controllerlist.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(main);
    			destroy_component(controllerlist);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('App', slots, []);
    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<App> was created with unknown prop '${key}'`);
    	});

    	$$self.$capture_state = () => ({ ControllerList });
    	return [];
    }

    class App extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance, create_fragment, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "App",
    			options,
    			id: create_fragment.name
    		});
    	}
    }

    const app = new App({
        target: document.body,
        props: {}
    });

    return app;

})();
//# sourceMappingURL=bundle.js.map
