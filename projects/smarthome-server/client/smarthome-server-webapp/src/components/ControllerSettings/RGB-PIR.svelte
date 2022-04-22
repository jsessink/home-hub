<script lang="ts">
  import { onMount } from "svelte";

  import type { RGBSettings } from "../../interface/rgb-settings.interface";
  import { hexToRGBA, rgbToHex } from "../../util/hex-to-rgba";

  export let controllerIP;

  let settingsFetch = getSettings();

  async function getSettings() {
    const res = await fetch(`/api/proxy/get-current-rgb-settings?ip=${controllerIP}`);
    const body: RGBSettings = await res.json();

    // Check if ok and even just a single prop exists to show data was actually sent
    if (res.ok && body.hasOwnProperty('r')) {
      return {
        hex: rgbToHex(body.r, body.g, body.b),
        ...body
      }
    }
  }

  onMount(() => {});

  const onSettingChange = e => {
    switch (e.target.dataset.setting) {
      case 'color':
        return colorChosen(e);
    
      default:
        const element = e.target as HTMLInputElement;
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

  async function settingChange(settingName: string, value: string | boolean | number) {
    const res = await fetch(
      `/api/proxy/rgb-setting-change?ip=${controllerIP}&setting-name=${settingName}&setting-value=${value}`,
      {
        method: "GET",
      }
    );

    await res.json();
  }
</script>

<div class="rgb-picker">
  {#await settingsFetch}
    <p>... Getting online status and settings ...</p>
  {:then rgbSettings}
    <div class="card">
      <h2>{rgbSettings.deviceName}</h2>

      <label>
        <span>Color</span>
        <input data-setting="color" type="color" value={rgbSettings.hex} on:change={onSettingChange} />
      </label>
      <label>
        <span>Always On?</span>
        <input data-setting="always-on" type="checkbox" checked={rgbSettings.alwaysOn} on:change={onSettingChange} />
      </label>
      <label>
        <span>Accidental Trip Delay (ms):</span>
        <input data-setting="acc-delay" type="number" value={rgbSettings.accidentalTripDelay} on:change={onSettingChange} />
      </label>
      <label>
        <span>Duration On (ms):</span>
        <input data-setting="duration-on" type="number" value={rgbSettings.durationOn} on:change={onSettingChange} />
      </label>
      <label>
        <span>Fade In Speed (ms):</span>
        <input data-setting="in-speed" type="number" value={rgbSettings.fadeInSpeed} on:change={onSettingChange} />
      </label>
      <label>
        <span>Fade Out Speed (ms):</span>
        <input data-setting="out-speed" type="number" value={rgbSettings.fadeOutSpeed} on:change={onSettingChange} />
      </label>
    </div>
  {:catch error}
    {controllerIP} Offline
  {/await}
</div>

<style lang="scss">
  .rgb-picker > .card {
    display: flex;
    flex-direction: column;
    text-align: left;

    background-color: #2d3434;
    border: 1px solid orange;
    border-radius: 10px;
    padding: 2rem;

    h2 {
      text-align: center;
      margin-top: 0;
    }

    label {
      display: flex;
      align-items: center;
      justify-content: space-between;

      padding: 0.5rem 0;

      > span {
        margin-right: 1rem;
      }

      > input {
        padding: 5px;
        
        outline: none;
        background: #e9f3f4;
        border: 1px solid #b9d5d6;
        border-radius: 4px;

        &[type="color"] {
          background: none;
          outline: none;
          padding: 0;
          border: none;
        }
      }
    }
  }
</style>
