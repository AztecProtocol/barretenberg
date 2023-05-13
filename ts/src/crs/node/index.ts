import { NetCrs } from '../net_crs.js';
import { FileCrs } from './file_crs.js';

/**
 * Generic CRS finder utility class.
 */
export class Crs {
  private crs: FileCrs | NetCrs;

  constructor(
    /**
     * The number of circuit gates.
     */
    public readonly numPoints: number,
  ) {
    this.crs = FileCrs.defaultExists() ? new FileCrs(numPoints) : new NetCrs(numPoints);
  }

  static async new(numPoints: number) {
    const crs = new Crs(numPoints);
    await crs.init();
    return crs;
  }

  /**
   * Read CRS from our chosen source.
   */
  async init() {
    await this.crs.init();
  }

  /**
   * G1 points data for prover key.
   * @returns The points data.
   */
  getG1Data(): Uint8Array {
    return this.crs.getG1Data();
  }

  /**
   * G2 points data for verification key.
   * @returns The points data.
   */
  getG2Data(): Uint8Array {
    return this.crs.getG2Data();
  }
}
