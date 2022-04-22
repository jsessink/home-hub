<script lang="ts">
	import ControllerList from './components/ControllerList.svelte';
</script>

<main>
	<h1>Sessink Home Hub</h1>
	<ControllerList />
</main>

<style lang="scss">
  :global(body) {
    display: flex;
    flex-direction: column;

    margin: 0;
  
    background: #182829;
    color: #e8f2fc;

    font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;
  }
	main {
		text-align: center;
		margin: 0 auto;
	}
	h1 {
    font-size: 3em;
    font-weight: 100;
	}
  
  h1
  /*h2,*/
  /*h3,*/
  /*h4,*/
  /*h5*/
  {
    color: #fefff7;
    text-transform: uppercase;
    font-family: Cambria, Cochin, Georgia, Times, 'Times New Roman', serif;
  }

	@media (min-width: 640px) {
		main {
			max-width: none;
		}
	}
</style>
