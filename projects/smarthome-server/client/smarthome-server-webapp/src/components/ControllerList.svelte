<script lang="ts">
    import { onMount, createEventDispatcher } from 'svelte';

    import AddNewControllerDialog from './AddNewControllerDialog.svelte';

    let addNewDialogOpen = false;

    onMount(() => {
        getControllersList();
    });

    // Get the list from the local hub
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

    function onAddNewClick() {
        addNewDialogOpen = true;
    }
</script>

<div class="controller-list-container">
    <!-- {#await promise then value}
        <p>the value is {value}</p>
    {/await} -->
</div>

{#if addNewDialogOpen}
<AddNewControllerDialog />
{/if}

<button on:click={ onAddNewClick }>Add New Controller</button>

<style>

</style>
