/** @file
 * @brief ACS Measurement Polyline API
 * @author Teledyne FLIR 
 * @copyright Copyright 2025: Teledyne FLIR 
 */

#ifndef ACS_MEASUREMENT_POLYLINE_H
#define ACS_MEASUREMENT_POLYLINE_H

#include <acs/common.h>
#include <acs/measurement_area.h>
#include <acs/measurement_marker.h>
#include <acs/measurement_shape.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
    /** @struct ACS_MeasurementPolyline
        @brief Represents a polyline shaped measurement area.
        @see   Single polyline:
        @see   ACS_Measurements_addPolyline
        @see   ACS_Measurements_findPolyline
        @see   ACS_Measurements_movePolyline
        @see   ACS_Measurements_removePolyline
        @see   Multiple polylines:
        @see   ACS_Measurements_getAllPolylines
        @see   ACS_ListMeasurementPolyline
    */
    typedef struct ACS_MeasurementPolyline_ ACS_MeasurementPolyline;

    /** @brief Set the location and shape of the polyline.
     *
     *  @param polyline     The polyline object.
     *  @param points       A list with the polyline's points (vertices) in 2D coordinates.
     *
     *  @remarks        Coordinates should be positive values and within the image. A minimum of 2 points is required to define a polyline.
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API void ACS_MeasurementPolyline_setPolyline(ACS_MeasurementPolyline* polyline, const ACS_ListPoint* points);

    /** @brief Move the location of the polyline with a given offset. All points (vertices) will be offset with the provided x-axis and y-axis offset value.
     *
     *  @param polyline     The polyline object.
     *  @param xOffset      The x-offset to move the polyline, negative values will move it left, positive values will move it right.
     *  @param yOffset      The y-offset to move the polyline, negative values will move it up, positive values will move it down.
     *
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API void ACS_MeasurementPolyline_offsetPolyline(ACS_MeasurementPolyline* polyline, int xOffset, int yOffset);

    /** @brief Get the points that defines the location and shape of the polyline.
     *
     *  @param polyline     The polyline object.
     *  @return             A list with the polyline's points (vertices) in 2D coordinates.
     * 
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API ACS_ListPoint* ACS_MeasurementPolyline_getPolylinePoints(const ACS_MeasurementPolyline* polyline);

    /** @brief Get the area aspect of the measurement.
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API const ACS_MeasurementArea* ACS_MeasurementPolyline_asMeasurementArea(const ACS_MeasurementPolyline* polyline);

    /** @brief Get the marker aspect of the measurement.
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API const ACS_MeasurementMarker* ACS_MeasurementPolyline_asMeasurementMarker(const ACS_MeasurementPolyline* polyline);

    /** @brief Get the shape aspect of the measurement.
     *  @relatesalso    ACS_MeasurementPolyline
     */
    ACS_API const ACS_MeasurementShape* ACS_MeasurementPolyline_asMeasurementShape(const ACS_MeasurementPolyline* polyline);

    /** @brief Gets the local thermal parameters for the polyline.
     *  @relatesalso    ACS_LocalThermalParameters
     */
    ACS_API ACS_LocalThermalParameters* ACS_MeasurementPolyline_getLocalThermalParameters(const ACS_MeasurementPolyline* polyline);

#ifdef __cplusplus
}
#endif

#endif // ACS_MEASUREMENT_POLYLINE_H
