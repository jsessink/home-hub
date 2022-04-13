const isValidHex = (hex: string) => /^#([A-Fa-f0-9]{3,4}){1,2}$/.test(hex);

const getChunksFromString = (st: string, chunkSize: number) => st.match(new RegExp(`.{${chunkSize}}`, "g"));

const convertHexUnitTo256 = (hexStr: string) => parseInt(hexStr.repeat(2 / hexStr.length), 16);

const getAlphaFloat = (a: number, alpha: number) => {
    if (typeof a !== "undefined") {
      return a / 255;
    }

    if ((typeof alpha != "number") || alpha <0 || alpha >1) {
      return 1;
    }

    return alpha;
};

export const hexToRGBA = (hex: string, alpha?: number) => {
    if (!isValidHex(hex)) {
      throw new Error("Invalid HEX")
    }

    const chunkSize = Math.floor((hex.length - 1) / 3);
    const hexArr = getChunksFromString(hex.slice(1), chunkSize);

    if (!hexArr) {
      throw new Error('Cannot obtain hex information');
    }
    
    const [r, g, b, a] = hexArr.map(convertHexUnitTo256);
    const alphaFloat = alpha ? getAlphaFloat(a, alpha) : 1;

    return { r, g, b, a: alphaFloat };
};

export const rgbToHex = (r, g, b) => {
  return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
};
