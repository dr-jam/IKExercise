using UnityEngine;
using System.Collections;

public class CameraController : MonoBehaviour
{
    public Camera camera;
    public Transform target;

    void Update()
    {
        if (Input.GetMouseButtonDown(0))
        {
            RaycastHit hit;
            Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
            if (Physics.Raycast(ray, out hit, 100.0f))
            {
                target.position = hit.point;
            }
        }
    }
}