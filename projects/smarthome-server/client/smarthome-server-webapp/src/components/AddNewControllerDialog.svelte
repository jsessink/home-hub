<script lang="ts">
    import { addNewControllerForm, AddNewControllerFormData } from '../store';

    async function onConfirmNewController() {
        const res = await fetch(`/api/add-controller`, {
			method: 'POST',
			body: JSON.stringify($addNewControllerForm as AddNewControllerFormData)
		});

        const text = await res.text();

        if (res.ok) {
            console.log(text);
			return text;
		} else {
			throw new Error(text);
		}
    }
</script>

<dialog class="add-new-controller-dialog">
    <h2>Please fill in the controller settings</h2>
    <form method="dialog">
        <p>
            <label>Controller Hostname (the name that shows up in your WiFi for the board):
                <input type="text" bind:value={$addNewControllerForm.controllerHostname} />
            </label>
        </p>
        <p>
            <label>Controller IP Address (only use if hostname doesn't work):
                <input type="text" bind:value={$addNewControllerForm.controllerIP} />
            </label>
        </p>
        <p>
            <label>Controller Type:
                <select bind:value={$addNewControllerForm.controllerType}>
                    <option value="pir-rgb">Motion sensor on RGB LED Strip</option>                    
                </select>
            </label>
        </p>
        <p>
            <label>Default Color:
                <input type="color" bind:value={$addNewControllerForm.defaultColor} />
            </label>
        </p>
        <p>
            <label>Always on? (disable motion detection / only use light switch):
                <input type="checkbox" bind:value={$addNewControllerForm.alwaysOn} />
            </label>
        </p>
        <p>
            <label>Accidental motion trip delay (in ms -> Waits for continuous motion for set time before triggering):
                <input type="number" bind:value={$addNewControllerForm.accidentalTripDelay} />
            </label>
        </p>
        <p>
            <label>Default duration on after motion lost (0 = always/infinite):
                <input type="number" bind:value={$addNewControllerForm.durationOn} />
            </label>
        </p>
        <p>
            <label>Fade speed in (in ms):
                <input type="number" bind:value={$addNewControllerForm.fadeInSpeed} />
            </label>
        </p>
        <p>
            <label>Fade speed out (in ms):
                <input type="number" bind:value={$addNewControllerForm.fadeOutSpeed} />
            </label>
        </p>
        <div>
            <button id="confirm" value="default" on:click={ onConfirmNewController }>Confirm</button>
            <button value="cancel">Cancel</button>
        </div>
    </form>
</dialog>

<style type="scss">
    dialog {
        // Default to being visible, since the entire dialog is conditioned already
        display: block;

        width: 80%;
        min-height: 80%;

        position: fixed;
        top: 10%;
        left: 0;
        
        box-shadow: -1px 3px 8px 1px #686878;
    }
</style>
