using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class Bezier : MonoBehaviour
{
    [SerializeField] private Color curveColor = Color.white;
    [SerializeField] private Bezier endKnot = null;
    public bool connectedTo;
    [SerializeField] private GameObject[] controlPoints = new GameObject[2];
    [Range(1, 100)] [SerializeField] int numSegments = 25;

    [ExecuteInEditMode]
    private void OnDrawGizmos()
    {
        updateControlPoints();
        setColors();
        drawControlPoints();
        if(endKnot != null)
        {
            drawCurve();
        }
    }

    void updateControlPoints()
    {
        if (endKnot != null)
        {
            endKnot.connectedTo = true;
            endKnot.controlPoints[1].SetActive(true);
            controlPoints[1].SetActive(true);
        }
        else
        {
            controlPoints[0].SetActive(false);
        }
        if (!connectedTo)
        {
            controlPoints[1].SetActive(false);
        }
    }
    void setColors()
    {
        Gizmos.color = curveColor;
    }
    void drawControlPoints()
    {
        Gizmos.DrawSphere(transform.position, 0.5f);
        Gizmos.DrawSphere(controlPoints[0].transform.position, 0.25f);
        Gizmos.DrawSphere(controlPoints[1].transform.position, 0.25f);
    }
    void drawCurve()
    {
        if (numSegments == 1)
        {
            Gizmos.DrawLine(transform.position, endKnot.transform.position);
        }
        else
        {
            Vector3[] curvePoints = new Vector3[numSegments - 1];
            for (int i = 0; i < curvePoints.Length; i++)
            {
                float t = (float)(i + 1) / numSegments;
                curvePoints[i] = getPointOnCurveT(t);
            }
            if (curvePoints.Length > 0)
            {
                Gizmos.DrawLine(transform.position, curvePoints[0]);
                for (int i = 0; i < curvePoints.Length - 1; i++)
                {
                    Gizmos.DrawLine(curvePoints[i], curvePoints[i + 1]);
                }
                Gizmos.DrawLine(curvePoints[curvePoints.Length - 1], endKnot.transform.position);
            }
        }
    }

    public float CalculateArcLength()
    {
        float lengthSum = 0;
        if (numSegments == 1)
        {
            lengthSum = Vector3.Distance(transform.position, endKnot.transform.position);
        }
        else
        {
            Vector3[] curvePoints = new Vector3[numSegments - 1];
            for (int i = 0; i < curvePoints.Length; i++)
            {
                float t = (float)(i + 1) / numSegments;
                curvePoints[i] = getPointOnCurveT(t);
            }
            if (curvePoints.Length > 0)
            {
                lengthSum += Vector3.Distance(transform.position, curvePoints[0]);
                for (int i = 0; i < curvePoints.Length - 1; i++)
                {
                    lengthSum += Vector3.Distance(curvePoints[i], curvePoints[i + 1]);
                }
                lengthSum += Vector3.Distance(curvePoints[curvePoints.Length - 1], endKnot.transform.position);
            }
        }
        return lengthSum;
    }
    public Vector3 getPointOnCurveT(float t)
    {
        Vector3 p0 = transform.position;
        Vector3 p1 = controlPoints[0].transform.position;
        Vector3 p2 = endKnot.controlPoints[1].transform.position;
        Vector3 p3 = endKnot.transform.position;
        Vector3 point = new Vector3();
        point = p0 + t * (-3 * p0 + 3 * p1) + t * t * (3 * p0 - 6 * p1 + 3 * p2) + t * t * t * (-p0 + 3 * p1 - 3 * p2 + p3);
        return point;
    }
    public Vector3 getPointOnCurveDist(float dist) 
    {
        Vector3 point = new Vector3();
        point = getPointOnCurveT(dist/CalculateArcLength());
        return point;
    }
}
