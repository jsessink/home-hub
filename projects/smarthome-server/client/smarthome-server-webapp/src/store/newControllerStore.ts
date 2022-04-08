import { Writable, writable } from 'svelte/store';

export interface AddNewControllerFormData {
  controllerHostname: string;
  controllerIP: string;
  controllerType: string;
  defaultColor: string;
  alwaysOn: boolean;
  accidentalTripDelay: number;
  durationOn: number;
  fadeInSpeed: number;
  fadeOutSpeed: number;
}

export const addNewControllerForm: Writable<AddNewControllerFormData> = 
  writable({
    controllerHostname: '',
    controllerIP: '0.0.0.0',
    controllerType: 'pir-rgb',
    defaultColor: '#FFFFFF',
    alwaysOn: false,
    accidentalTripDelay: 500,
    durationOn: 5000,
    fadeInSpeed: 500,
    fadeOutSpeed: 750,
  });
