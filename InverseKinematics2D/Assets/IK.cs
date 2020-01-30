using UnityEngine;

public class IK : MonoBehaviour
{
    public int IKChainLength = 4;
    public Transform Target;

    public int IKIter = 5;

    public float stoppingDelta = 0.01f;

    [Range(0, 1)]
    public float springStrength = 1f;


    protected Transform joint;
    protected Transform[] jointArray;
    protected Vector3[] jointPosArray;
    protected float[] jointLengthArray; 
    protected float TotalRigLength;
    protected Vector3[] InitDirection;
    protected Quaternion[] InitRot;
    protected Quaternion TargetRot;

    void Awake()
    {
        //initial array
        jointArray = new Transform[IKChainLength + 1];
        jointPosArray = new Vector3[IKChainLength + 1];
        jointLengthArray = new float[IKChainLength];
        InitDirection = new Vector3[IKChainLength + 1];
        InitRot = new Quaternion[IKChainLength + 1];

        //find root
        joint = transform;
        for (var i = 0; i <= IKChainLength; i++)
        {
            joint = joint.parent;
        }
        var current = transform;
        TotalRigLength = 0;
        for (var i = jointArray.Length - 1; i >= 0; i--)
        {
            jointArray[i] = current;
            InitRot[i] = GetRot(current);

            if (i == jointArray.Length - 1)
            {
                InitDirection[i] = GetPos(Target) - GetPos(current);
            }
            else
            {
                InitDirection[i] = GetPos(jointArray[i + 1]) - GetPos(current);
                jointLengthArray[i] = InitDirection[i].magnitude;
                TotalRigLength += jointLengthArray[i];
            }
            current = current.parent;
        }
    }

    void LateUpdate()
    {
        var targetPos = GetPos(Target);
        //get joint positions
        for (int i = 0; i < jointArray.Length; i++)
        {
            
        }

        //Is it possible to reach the target?
        var targetDistance = targetPos - GetPos(jointArray[0]);
        if ((targetDistance).magnitude >= TotalRigLength)
        {
            //Move entire rig towards target
        }
        else
        {
            for (int i = 0; i < this.jointPosArray.Length - 1; i++)
            {
                this.jointPosArray[i + 1] = Vector3.Lerp(this.jointPosArray[i + 1], this.jointPosArray[i] + InitDirection[i], springStrength);
            }

            for (int i = 0; i < IKIter; i++)
            {
                // Do back calculation
                Backwards(targetPos);
                // Do forward calculation
                Forwards(targetPos);
                // Should rig stop?
                var rigDistance = this.jointPosArray[this.jointPosArray.Length - 1] - targetPos;
                if ((rigDistance).magnitude < stoppingDelta)
                {
                    //Stop the rig if close enough
                }
            }
        }

        //set joint positions
        for (int i = 0; i < this.jointPosArray.Length; i++)
        {

        }
    }

    private void Backwards(Vector3 targetPos)
    {

    }
    private void Forwards(Vector3 targetPos)
    {

    }
    private Quaternion GetRot(Transform current)
    {
        if (joint == null)
        {
            return current.rotation;
        }
        else
        {
            return Quaternion.Inverse(current.rotation) * joint.rotation;
        }
    }

    private void SetRot(Transform current, Quaternion rotation)
    {
        if (joint == null)
        {
            current.rotation = rotation;
        }
        else
        {
            current.rotation = joint.rotation * rotation;
        }
    }
    private Vector3 GetPos(Transform current)
    {
        if (joint == null)
        {
            return current.position;
        }
        else
        {
            return Quaternion.Inverse(joint.rotation) * (current.position - joint.position);
        }
    }

    private void SetPos(Transform current, Vector3 position)
    {
        if (joint == null)
        {
            current.position = position;
        }
        else
        {
            current.position = joint.rotation * position + joint.position;
        }
    }
}
