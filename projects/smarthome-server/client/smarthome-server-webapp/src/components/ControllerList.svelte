<script lang="ts">
    import { onMount, createEventDispatcher } from 'svelte';

    import RgbPir from './ControllerSettings/RGB-PIR.svelte';
    import AddNewControllerDialog from './AddNewControllerDialog.svelte';

    let addNewDialogOpen = false;

    const controllers$: Promise<{ controllerIP: string, controllerType: string; }[]> = getControllersList();

    onMount(() => {});

    // Get the list from the local hub
    async function getControllersList() {
        const res = await fetch(`/api/get-controller-list`);
        const body = await res.json();

        const controllers = [];

        if (res.ok) {
            Object.keys(body).map((key, index) => {
                controllers.push({
                    controllerIP: key,
                    controllerType: body[key][0]
                });

                console.log(controllers);
            });
		}
        
	    return controllers;
    }
</script>

<div class="controller-list-container">
    <!-- {#await promise then value}
        <p>the value is {value}</p>
    {/await} -->

    {#await controllers$ then controllers}
        {#each controllers as controller}
            {#if controller.controllerType == '1' && controller.controllerIP == '192.168.4.2'}
                <RgbPir controllerIP={controller.controllerIP} />
            {/if}
        {/each}
    {/await}
</div>

<style>

</style>
