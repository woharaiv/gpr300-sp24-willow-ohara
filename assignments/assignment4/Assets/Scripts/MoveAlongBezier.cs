using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MoveAlongBezier : MonoBehaviour
{
    [SerializeField] private Bezier[] knots;
    [SerializeField] private float t = 0;
    [SerializeField] private float velocity = 1;
    private float totalLength;

    Vector3 lastPos;

    private void Start()
    {
        t = 0;
    }

    void Update()
    {
        //Calculate total length of curcuit (have to do this every frame because control points can be moved at any point)
        totalLength = 0;
        foreach (var curve in knots)
        {
            totalLength += curve.CalculateArcLength();
        }
        //Movement
        t += velocity * Time.deltaTime;
        //Loop to beginning if end of circuit has been passed
        t %= totalLength;
        
        int activeCurve = 0;
        float lengthOfPassedCurves = 0;
        foreach(var curve in knots) 
        {
            //If the total distance is greater this curve and all the curves already passed, then this curve has already been passed.
            if (t > lengthOfPassedCurves + curve.CalculateArcLength())
            {
                activeCurve++;
                lengthOfPassedCurves += curve.CalculateArcLength();
            }
            else //If the total distance doesn't pass the end of this curve, then it's the curve that the cube is currently on.
                break;
        }
        transform.position = knots[activeCurve].getPointOnCurveDist(t - lengthOfPassedCurves);
        Debug.Log(Vector3.Distance(lastPos, transform.position));
        transform.LookAt(lastPos);
        lastPos = transform.position;
    }
}
