<script lang="ts">
    import { onMount, createEventDispatcher } from 'svelte';
    import { hexToRGBA } from '../util/hex-to-rgba';

    onMount(() => {
        const input = document.querySelector('input');
        input?.addEventListener('change', colorChosen);
    });

    // Dispatcher for color change
    const dispatch = createEventDispatcher();

    function colorChosen(e: InputEvent) {
        const rgba = hexToRGBA((e.target as HTMLInputElement)?.value);
        dispatch('colorChange', rgba);
    }
</script>

<div class="rgba-picker">
    <input type="color" />
</div>

<style>

</style>
