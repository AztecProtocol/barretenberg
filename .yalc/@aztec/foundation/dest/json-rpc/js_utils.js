// Make sure this property was not inherited
/**
 * Does this own the property?
 * @param obj - An object.
 * @param method - A property name.
 */
export const hasOwnProperty = (obj, propertyName) => Object.prototype.hasOwnProperty.call(obj, propertyName);
export const assert = (x, err) => {
    if (!x) {
        throw new Error(err);
    }
};
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoianNfdXRpbHMuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi9zcmMvanNvbi1ycGMvanNfdXRpbHMudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsNENBQTRDO0FBRTVDOzs7O0dBSUc7QUFDSCxNQUFNLENBQUMsTUFBTSxjQUFjLEdBQUcsQ0FBQyxHQUFRLEVBQUUsWUFBb0IsRUFBRSxFQUFFLENBQy9ELE1BQU0sQ0FBQyxTQUFTLENBQUMsY0FBYyxDQUFDLElBQUksQ0FBQyxHQUFHLEVBQUUsWUFBWSxDQUFDLENBQUM7QUFFMUQsTUFBTSxDQUFDLE1BQU0sTUFBTSxHQUFHLENBQUMsQ0FBTSxFQUFFLEdBQVcsRUFBRSxFQUFFO0lBQzVDLElBQUksQ0FBQyxDQUFDLEVBQUU7UUFDTixNQUFNLElBQUksS0FBSyxDQUFDLEdBQUcsQ0FBQyxDQUFDO0tBQ3RCO0FBQ0gsQ0FBQyxDQUFDIn0=