<script lang="ts">
	import RGBAPicker from './components/RGBAPicker.svelte';

	async function onColorChange(e: CustomEvent<{ detail: { r: number, g: number, b: number, a: number } }>) {
		const res = await fetch('/api/rgb-change', {
			method: 'POST',
			body: JSON.stringify(e.detail)
		})
		
		const json = await res.json()
		console.log(JSON.stringify(json));
	}
</script>

<main>
	<h1>Hello there!</h1>
	<h2>Choose a color for your lights</h2>

	<RGBAPicker on:colorChange={ onColorChange } />
</main>

<style>
	main {
		text-align: center;
		padding: 1em;
		max-width: 240px;
		margin: 0 auto;
	}
	h1 {
		color: #ff3e00;
		text-transform: uppercase;
		font-size: 4em;
		font-weight: 100;
	}
	@media (min-width: 640px) {
		main {
			max-width: none;
		}
	}
</style>
