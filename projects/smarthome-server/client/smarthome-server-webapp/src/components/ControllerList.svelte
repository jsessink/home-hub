<script lang="ts">
  import { onMount } from "svelte";
  import { ControllerType } from "../common/ControllerSettings";

  import RgbPir from "./ControllerSettings/RGB-PIR.svelte";

  const controllers$: Promise<
    { controllerIP: string; controllerType: string }[]
  > = getControllersList();

  onMount(() => {});

  // Get the list from the local hub
  async function getControllersList() {
    const res = await fetch(`/api/get-controller-list`);
    const body = await res.json();

    if (res.ok) {
      return body;
    } else {
      throw 'Something went wrong';
    }
  }
</script>

<div class="controller-list-container">
  {#await controllers$ then controllers}
    {#each controllers as controller}
      {#if controller.controllerType === ControllerType.PIR_RGB.toString()}
        <RgbPir controllerIP={controller.controllerIP} />
      {/if}
    {/each}
  {/await}
</div>

<style>
</style>
