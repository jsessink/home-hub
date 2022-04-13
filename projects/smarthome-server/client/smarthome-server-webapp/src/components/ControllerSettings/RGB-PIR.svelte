<script lang="ts">
  import { onMount } from "svelte";
  import { hexToRGBA, rgbToHex } from "../../util/hex-to-rgba";

  export let controllerIP;

  let currentColorFetch = getCurrentColor();

  async function getCurrentColor() {
    const res = await fetch(`/api/proxy/get-current-rgb?ip=${controllerIP}`);
    const body = await res.json();

    if (res.ok && body.hasOwnProperty('r')) {
      return rgbToHex(body.r, body.g, body.b);
    }
  }

  onMount(() => {});

  const onChange = e => colorChosen(e);
  async function colorChosen(e: Event) {
    const rgb = hexToRGBA((e.target as HTMLInputElement)?.value);
    const res = await fetch(
      `/api/proxy/rgb?ip=${controllerIP}&r=${rgb.r}&g=${rgb.g}&b=${rgb.b}`,
      {
        method: "GET",
      }
    );

    await res.json();
  }
</script>

<div class="RGB-picker">
  {#await currentColorFetch}
    <p>... Getting online status and settings ...</p>
  {:then currentColor}
    <input type="color" value={currentColor} on:change={onChange} />
  {:catch error}
    {controllerIP} Offline
  {/await}
</div>

<style>
</style>
