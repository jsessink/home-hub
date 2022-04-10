<script lang="ts">
    import { onMount, createEventDispatcher } from 'svelte';
    import { hexToRGBA } from '../../util/hex-to-rgba';

    export let controllerIP;

    onMount(() => {
        const input = document.querySelector('input');
        input?.addEventListener('change', colorChosen);
    });

    // Dispatcher for color change
    // const dispatch = createEventDispatcher();

    async function colorChosen(e: InputEvent) {
        const rgb = hexToRGBA((e.target as HTMLInputElement)?.value);
        // dispatch('colorChange', RGB);

        console.log('Color changed! ' + rgb);

        const res = await fetch(`/api/proxy-change/rgb?ip=${controllerIP}&r=${rgb.r}&g=${rgb.g}&b=${rgb.b}`, {
			method: 'GET',
		})
		
		const json = await res.json()
		console.log(JSON.stringify(json));
    }
</script>

<div class="RGB-picker">
    <input type="color" />
</div>

<style>

</style>
