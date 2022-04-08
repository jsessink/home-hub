
(function(l, r) { if (!l || l.getElementById('livereloadscript')) return; r = l.createElement('script'); r.async = 1; r.src = '//' + (self.location.host || 'localhost').split(':')[0] + ':35729/livereload.js?snipver=1'; r.id = 'livereloadscript'; l.getElementsByTagName('head')[0].appendChild(r) })(self.document);
var app = (function () {
    'use strict';

    function noop() { }
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
    function validate_store(store, name) {
        if (store != null && typeof store.subscribe !== 'function') {
            throw new Error(`'${name}' is not a store with a 'subscribe' method`);
        }
    }
    function subscribe(store, ...callbacks) {
        if (store == null) {
            return noop;
        }
        const unsub = store.subscribe(...callbacks);
        return unsub.unsubscribe ? () => unsub.unsubscribe() : unsub;
    }
    function component_subscribe(component, store, callback) {
        component.$$.on_destroy.push(subscribe(store, callback));
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
    function element(name) {
        return document.createElement(name);
    }
    function text(data) {
        return document.createTextNode(data);
    }
    function space() {
        return text(' ');
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
    function to_number(value) {
        return value === '' ? null : +value;
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function set_input_value(input, value) {
        input.value = value == null ? '' : value;
    }
    function select_option(select, value) {
        for (let i = 0; i < select.options.length; i += 1) {
            const option = select.options[i];
            if (option.__value === value) {
                option.selected = true;
                return;
            }
        }
        select.selectedIndex = -1; // no option should be selected
    }
    function select_value(select) {
        const selected_option = select.querySelector(':checked') || select.options[0];
        return selected_option && selected_option.__value;
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

    const subscriber_queue = [];
    /**
     * Create a `Writable` store that allows both updating and reading by subscription.
     * @param {*=}value initial value
     * @param {StartStopNotifier=}start start and stop notifications for subscriptions
     */
    function writable(value, start = noop) {
        let stop;
        const subscribers = new Set();
        function set(new_value) {
            if (safe_not_equal(value, new_value)) {
                value = new_value;
                if (stop) { // store is ready
                    const run_queue = !subscriber_queue.length;
                    for (const subscriber of subscribers) {
                        subscriber[1]();
                        subscriber_queue.push(subscriber, value);
                    }
                    if (run_queue) {
                        for (let i = 0; i < subscriber_queue.length; i += 2) {
                            subscriber_queue[i][0](subscriber_queue[i + 1]);
                        }
                        subscriber_queue.length = 0;
                    }
                }
            }
        }
        function update(fn) {
            set(fn(value));
        }
        function subscribe(run, invalidate = noop) {
            const subscriber = [run, invalidate];
            subscribers.add(subscriber);
            if (subscribers.size === 1) {
                stop = start(set) || noop;
            }
            run(value);
            return () => {
                subscribers.delete(subscriber);
                if (subscribers.size === 0) {
                    stop();
                    stop = null;
                }
            };
        }
        return { set, update, subscribe };
    }

    const addNewControllerForm = writable({
        controllerHostname: '',
        controllerIP: '0.0.0.0',
        controllerType: 'pir-rgb',
        defaultColor: '#FFFFFF',
        alwaysOn: false,
        accidentalTripDelay: 500,
        durationOn: 5000,
        fadeInSpeed: 500,
        fadeOutSpeed: 750,
    });

    /* src\components\AddNewControllerDialog.svelte generated by Svelte v3.46.5 */

    const { Error: Error_1$1, console: console_1$1 } = globals;
    const file$2 = "src\\components\\AddNewControllerDialog.svelte";

    function create_fragment$2(ctx) {
    	let dialog;
    	let h2;
    	let t1;
    	let form;
    	let p0;
    	let label0;
    	let t2;
    	let input0;
    	let t3;
    	let p1;
    	let label1;
    	let t4;
    	let input1;
    	let t5;
    	let p2;
    	let label2;
    	let t6;
    	let select;
    	let option;
    	let t8;
    	let p3;
    	let label3;
    	let t9;
    	let input2;
    	let t10;
    	let p4;
    	let label4;
    	let t11;
    	let input3;
    	let t12;
    	let p5;
    	let label5;
    	let t13;
    	let input4;
    	let t14;
    	let p6;
    	let label6;
    	let t15;
    	let input5;
    	let t16;
    	let p7;
    	let label7;
    	let t17;
    	let input6;
    	let t18;
    	let p8;
    	let label8;
    	let t19;
    	let input7;
    	let t20;
    	let div;
    	let button0;
    	let t22;
    	let button1;
    	let mounted;
    	let dispose;

    	const block = {
    		c: function create() {
    			dialog = element("dialog");
    			h2 = element("h2");
    			h2.textContent = "Please fill in the controller settings";
    			t1 = space();
    			form = element("form");
    			p0 = element("p");
    			label0 = element("label");
    			t2 = text("Controller Hostname (the name that shows up in your WiFi for the board):\r\n                ");
    			input0 = element("input");
    			t3 = space();
    			p1 = element("p");
    			label1 = element("label");
    			t4 = text("Controller IP Address (only use if hostname doesn't work):\r\n                ");
    			input1 = element("input");
    			t5 = space();
    			p2 = element("p");
    			label2 = element("label");
    			t6 = text("Controller Type:\r\n                ");
    			select = element("select");
    			option = element("option");
    			option.textContent = "Motion sensor on RGB LED Strip";
    			t8 = space();
    			p3 = element("p");
    			label3 = element("label");
    			t9 = text("Default Color:\r\n                ");
    			input2 = element("input");
    			t10 = space();
    			p4 = element("p");
    			label4 = element("label");
    			t11 = text("Always on? (disable motion detection / only use light switch):\r\n                ");
    			input3 = element("input");
    			t12 = space();
    			p5 = element("p");
    			label5 = element("label");
    			t13 = text("Accidental motion trip delay (in ms -> Waits for continuous motion for set time before triggering):\r\n                ");
    			input4 = element("input");
    			t14 = space();
    			p6 = element("p");
    			label6 = element("label");
    			t15 = text("Default duration on after motion lost (0 = always/infinite):\r\n                ");
    			input5 = element("input");
    			t16 = space();
    			p7 = element("p");
    			label7 = element("label");
    			t17 = text("Fade speed in (in ms):\r\n                ");
    			input6 = element("input");
    			t18 = space();
    			p8 = element("p");
    			label8 = element("label");
    			t19 = text("Fade speed out (in ms):\r\n                ");
    			input7 = element("input");
    			t20 = space();
    			div = element("div");
    			button0 = element("button");
    			button0.textContent = "Confirm";
    			t22 = space();
    			button1 = element("button");
    			button1.textContent = "Cancel";
    			add_location(h2, file$2, 18, 4, 479);
    			attr_dev(input0, "type", "text");
    			add_location(input0, file$2, 22, 16, 678);
    			add_location(label0, file$2, 21, 12, 581);
    			add_location(p0, file$2, 20, 8, 564);
    			attr_dev(input1, "type", "text");
    			add_location(input1, file$2, 27, 16, 899);
    			add_location(label1, file$2, 26, 12, 816);
    			add_location(p1, file$2, 25, 8, 799);
    			option.__value = "pir-rgb";
    			option.value = option.__value;
    			add_location(option, file$2, 33, 20, 1152);
    			if (/*$addNewControllerForm*/ ctx[0].controllerType === void 0) add_render_callback(() => /*select_change_handler*/ ctx[4].call(select));
    			add_location(select, file$2, 32, 16, 1072);
    			add_location(label2, file$2, 31, 12, 1031);
    			add_location(p2, file$2, 30, 8, 1014);
    			attr_dev(input2, "type", "color");
    			add_location(input2, file$2, 39, 16, 1364);
    			add_location(label3, file$2, 38, 12, 1325);
    			add_location(p3, file$2, 37, 8, 1308);
    			attr_dev(input3, "type", "checkbox");
    			add_location(input3, file$2, 44, 16, 1584);
    			add_location(label4, file$2, 43, 12, 1497);
    			add_location(p4, file$2, 42, 8, 1480);
    			attr_dev(input4, "type", "number");
    			add_location(input4, file$2, 49, 16, 1840);
    			add_location(label5, file$2, 48, 12, 1716);
    			add_location(p5, file$2, 47, 8, 1699);
    			attr_dev(input5, "type", "number");
    			add_location(input5, file$2, 54, 16, 2066);
    			add_location(label6, file$2, 53, 12, 1981);
    			add_location(p6, file$2, 52, 8, 1964);
    			attr_dev(input6, "type", "number");
    			add_location(input6, file$2, 59, 16, 2245);
    			add_location(label7, file$2, 58, 12, 2198);
    			add_location(p7, file$2, 57, 8, 2181);
    			attr_dev(input7, "type", "number");
    			add_location(input7, file$2, 64, 16, 2426);
    			add_location(label8, file$2, 63, 12, 2378);
    			add_location(p8, file$2, 62, 8, 2361);
    			attr_dev(button0, "id", "confirm");
    			button0.value = "default";
    			add_location(button0, file$2, 68, 12, 2562);
    			button1.value = "cancel";
    			add_location(button1, file$2, 69, 12, 2665);
    			add_location(div, file$2, 67, 8, 2543);
    			attr_dev(form, "method", "dialog");
    			add_location(form, file$2, 19, 4, 532);
    			attr_dev(dialog, "class", "add-new-controller-dialog svelte-1tv3h5m");
    			add_location(dialog, file$2, 17, 0, 431);
    		},
    		l: function claim(nodes) {
    			throw new Error_1$1("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, dialog, anchor);
    			append_dev(dialog, h2);
    			append_dev(dialog, t1);
    			append_dev(dialog, form);
    			append_dev(form, p0);
    			append_dev(p0, label0);
    			append_dev(label0, t2);
    			append_dev(label0, input0);
    			set_input_value(input0, /*$addNewControllerForm*/ ctx[0].controllerHostname);
    			append_dev(form, t3);
    			append_dev(form, p1);
    			append_dev(p1, label1);
    			append_dev(label1, t4);
    			append_dev(label1, input1);
    			set_input_value(input1, /*$addNewControllerForm*/ ctx[0].controllerIP);
    			append_dev(form, t5);
    			append_dev(form, p2);
    			append_dev(p2, label2);
    			append_dev(label2, t6);
    			append_dev(label2, select);
    			append_dev(select, option);
    			select_option(select, /*$addNewControllerForm*/ ctx[0].controllerType);
    			append_dev(form, t8);
    			append_dev(form, p3);
    			append_dev(p3, label3);
    			append_dev(label3, t9);
    			append_dev(label3, input2);
    			set_input_value(input2, /*$addNewControllerForm*/ ctx[0].defaultColor);
    			append_dev(form, t10);
    			append_dev(form, p4);
    			append_dev(p4, label4);
    			append_dev(label4, t11);
    			append_dev(label4, input3);
    			set_input_value(input3, /*$addNewControllerForm*/ ctx[0].alwaysOn);
    			append_dev(form, t12);
    			append_dev(form, p5);
    			append_dev(p5, label5);
    			append_dev(label5, t13);
    			append_dev(label5, input4);
    			set_input_value(input4, /*$addNewControllerForm*/ ctx[0].accidentalTripDelay);
    			append_dev(form, t14);
    			append_dev(form, p6);
    			append_dev(p6, label6);
    			append_dev(label6, t15);
    			append_dev(label6, input5);
    			set_input_value(input5, /*$addNewControllerForm*/ ctx[0].durationOn);
    			append_dev(form, t16);
    			append_dev(form, p7);
    			append_dev(p7, label7);
    			append_dev(label7, t17);
    			append_dev(label7, input6);
    			set_input_value(input6, /*$addNewControllerForm*/ ctx[0].fadeInSpeed);
    			append_dev(form, t18);
    			append_dev(form, p8);
    			append_dev(p8, label8);
    			append_dev(label8, t19);
    			append_dev(label8, input7);
    			set_input_value(input7, /*$addNewControllerForm*/ ctx[0].fadeOutSpeed);
    			append_dev(form, t20);
    			append_dev(form, div);
    			append_dev(div, button0);
    			append_dev(div, t22);
    			append_dev(div, button1);

    			if (!mounted) {
    				dispose = [
    					listen_dev(input0, "input", /*input0_input_handler*/ ctx[2]),
    					listen_dev(input1, "input", /*input1_input_handler*/ ctx[3]),
    					listen_dev(select, "change", /*select_change_handler*/ ctx[4]),
    					listen_dev(input2, "input", /*input2_input_handler*/ ctx[5]),
    					listen_dev(input3, "change", /*input3_change_handler*/ ctx[6]),
    					listen_dev(input4, "input", /*input4_input_handler*/ ctx[7]),
    					listen_dev(input5, "input", /*input5_input_handler*/ ctx[8]),
    					listen_dev(input6, "input", /*input6_input_handler*/ ctx[9]),
    					listen_dev(input7, "input", /*input7_input_handler*/ ctx[10]),
    					listen_dev(button0, "click", /*onConfirmNewController*/ ctx[1], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*$addNewControllerForm*/ 1 && input0.value !== /*$addNewControllerForm*/ ctx[0].controllerHostname) {
    				set_input_value(input0, /*$addNewControllerForm*/ ctx[0].controllerHostname);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1 && input1.value !== /*$addNewControllerForm*/ ctx[0].controllerIP) {
    				set_input_value(input1, /*$addNewControllerForm*/ ctx[0].controllerIP);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1) {
    				select_option(select, /*$addNewControllerForm*/ ctx[0].controllerType);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1) {
    				set_input_value(input2, /*$addNewControllerForm*/ ctx[0].defaultColor);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1) {
    				set_input_value(input3, /*$addNewControllerForm*/ ctx[0].alwaysOn);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1 && to_number(input4.value) !== /*$addNewControllerForm*/ ctx[0].accidentalTripDelay) {
    				set_input_value(input4, /*$addNewControllerForm*/ ctx[0].accidentalTripDelay);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1 && to_number(input5.value) !== /*$addNewControllerForm*/ ctx[0].durationOn) {
    				set_input_value(input5, /*$addNewControllerForm*/ ctx[0].durationOn);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1 && to_number(input6.value) !== /*$addNewControllerForm*/ ctx[0].fadeInSpeed) {
    				set_input_value(input6, /*$addNewControllerForm*/ ctx[0].fadeInSpeed);
    			}

    			if (dirty & /*$addNewControllerForm*/ 1 && to_number(input7.value) !== /*$addNewControllerForm*/ ctx[0].fadeOutSpeed) {
    				set_input_value(input7, /*$addNewControllerForm*/ ctx[0].fadeOutSpeed);
    			}
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(dialog);
    			mounted = false;
    			run_all(dispose);
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
    	let $addNewControllerForm;
    	validate_store(addNewControllerForm, 'addNewControllerForm');
    	component_subscribe($$self, addNewControllerForm, $$value => $$invalidate(0, $addNewControllerForm = $$value));
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('AddNewControllerDialog', slots, []);

    	async function onConfirmNewController() {
    		const res = await fetch(`/api/add-controller`, {
    			method: 'POST',
    			body: JSON.stringify($addNewControllerForm)
    		});

    		const text = await res.text();

    		if (res.ok) {
    			console.log(text);
    			return text;
    		} else {
    			throw new Error(text);
    		}
    	}

    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1$1.warn(`<AddNewControllerDialog> was created with unknown prop '${key}'`);
    	});

    	function input0_input_handler() {
    		$addNewControllerForm.controllerHostname = this.value;
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input1_input_handler() {
    		$addNewControllerForm.controllerIP = this.value;
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function select_change_handler() {
    		$addNewControllerForm.controllerType = select_value(this);
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input2_input_handler() {
    		$addNewControllerForm.defaultColor = this.value;
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input3_change_handler() {
    		$addNewControllerForm.alwaysOn = this.value;
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input4_input_handler() {
    		$addNewControllerForm.accidentalTripDelay = to_number(this.value);
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input5_input_handler() {
    		$addNewControllerForm.durationOn = to_number(this.value);
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input6_input_handler() {
    		$addNewControllerForm.fadeInSpeed = to_number(this.value);
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	function input7_input_handler() {
    		$addNewControllerForm.fadeOutSpeed = to_number(this.value);
    		addNewControllerForm.set($addNewControllerForm);
    	}

    	$$self.$capture_state = () => ({
    		addNewControllerForm,
    		onConfirmNewController,
    		$addNewControllerForm
    	});

    	return [
    		$addNewControllerForm,
    		onConfirmNewController,
    		input0_input_handler,
    		input1_input_handler,
    		select_change_handler,
    		input2_input_handler,
    		input3_change_handler,
    		input4_input_handler,
    		input5_input_handler,
    		input6_input_handler,
    		input7_input_handler
    	];
    }

    class AddNewControllerDialog extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$2, create_fragment$2, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "AddNewControllerDialog",
    			options,
    			id: create_fragment$2.name
    		});
    	}
    }

    /* src\components\ControllerList.svelte generated by Svelte v3.46.5 */

    const { Error: Error_1, console: console_1 } = globals;
    const file$1 = "src\\components\\ControllerList.svelte";

    // (30:0) {#if addNewDialogOpen}
    function create_if_block(ctx) {
    	let addnewcontrollerdialog;
    	let current;
    	addnewcontrollerdialog = new AddNewControllerDialog({ $$inline: true });

    	const block = {
    		c: function create() {
    			create_component(addnewcontrollerdialog.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(addnewcontrollerdialog, target, anchor);
    			current = true;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(addnewcontrollerdialog.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(addnewcontrollerdialog.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(addnewcontrollerdialog, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block.name,
    		type: "if",
    		source: "(30:0) {#if addNewDialogOpen}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$1(ctx) {
    	let div;
    	let t0;
    	let t1;
    	let button;
    	let current;
    	let mounted;
    	let dispose;
    	let if_block = /*addNewDialogOpen*/ ctx[0] && create_if_block(ctx);

    	const block = {
    		c: function create() {
    			div = element("div");
    			t0 = space();
    			if (if_block) if_block.c();
    			t1 = space();
    			button = element("button");
    			button.textContent = "Add New Controller";
    			attr_dev(div, "class", "controller-list-container");
    			add_location(div, file$1, 23, 0, 577);
    			add_location(button, file$1, 33, 0, 782);
    		},
    		l: function claim(nodes) {
    			throw new Error_1("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			insert_dev(target, t0, anchor);
    			if (if_block) if_block.m(target, anchor);
    			insert_dev(target, t1, anchor);
    			insert_dev(target, button, anchor);
    			current = true;

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*onAddNewClick*/ ctx[1], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*addNewDialogOpen*/ ctx[0]) {
    				if (if_block) {
    					if (dirty & /*addNewDialogOpen*/ 1) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block(ctx);
    					if_block.c();
    					transition_in(if_block, 1);
    					if_block.m(t1.parentNode, t1);
    				}
    			} else if (if_block) {
    				group_outros();

    				transition_out(if_block, 1, 1, () => {
    					if_block = null;
    				});

    				check_outros();
    			}
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
    			if (detaching) detach_dev(div);
    			if (detaching) detach_dev(t0);
    			if (if_block) if_block.d(detaching);
    			if (detaching) detach_dev(t1);
    			if (detaching) detach_dev(button);
    			mounted = false;
    			dispose();
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
    	const text = await res.text();

    	if (res.ok) {
    		console.log(text);
    		return text;
    	} else {
    		throw new Error(text);
    	}
    }

    function instance$1($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('ControllerList', slots, []);
    	let addNewDialogOpen = false;

    	onMount(() => {
    		getControllersList();
    	});

    	function onAddNewClick() {
    		$$invalidate(0, addNewDialogOpen = true);
    	}

    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console_1.warn(`<ControllerList> was created with unknown prop '${key}'`);
    	});

    	$$self.$capture_state = () => ({
    		onMount,
    		AddNewControllerDialog,
    		addNewDialogOpen,
    		getControllersList,
    		onAddNewClick
    	});

    	$$self.$inject_state = $$props => {
    		if ('addNewDialogOpen' in $$props) $$invalidate(0, addNewDialogOpen = $$props.addNewDialogOpen);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [addNewDialogOpen, onAddNewClick];
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
    			h1.textContent = "Hello there!";
    			t1 = space();
    			create_component(controllerlist.$$.fragment);
    			attr_dev(h1, "class", "svelte-2x1evt");
    			add_location(h1, file, 4, 1, 103);
    			attr_dev(main, "class", "svelte-2x1evt");
    			add_location(main, file, 3, 0, 95);
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
